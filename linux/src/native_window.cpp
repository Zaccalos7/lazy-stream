#include "native_window.hpp"
#include "history_cleanup_service.hpp"
#include "live_rows.hpp"
#include "native_dashboard.hpp"
#include "native_theme.hpp"
#include "scene_service.hpp"
#include "version.hpp"
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFrame>
#include <QFutureWatcher>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QStylePainter>
#include <QTableWidget>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <algorithm>
#include <signal.h>
#include <unistd.h>

namespace orbis {
namespace {
class SidebarLanguagePicker : public QComboBox {
  protected:
    void paintEvent(QPaintEvent*) override {
        QStyleOptionComboBox option;
        initStyleOption(&option);
        if (property("compact").toBool())
            option.currentText.clear();
        QStylePainter painter(this);
        painter.drawComplexControl(QStyle::CC_ComboBox, option);
        painter.drawControl(QStyle::CE_ComboBoxLabel, option);
    }

  public:
    explicit SidebarLanguagePicker(QWidget* parent) : QComboBox(parent) {}
};

struct JobResult {
    Json value;
    std::string error;
};

// Il percorso canonico identifica la cartella anche quando il lancio usa un link simbolico.
QString activationSocket(const fs::path& root) {
    const auto canonical = QString::fromStdString(fs::canonical(root).string()).toUtf8();
    const auto hash = QCryptographicHash::hash(canonical, QCryptographicHash::Sha256).toHex();
    return QDir::tempPath() + "/orbis-" + QString::number(getuid()) + "-" +
           QString::fromLatin1(hash.left(32));
}

QString s(const Json& value) {
    return QString::fromStdString(value.str());
}

// I contatori non disponibili restano distinti da uno zero misurato.
QString formatMetric(double value, const QString& unit) {
    return value < 0 ? QString::fromUtf8("—") : QLocale().toString(value, 'f', 1) + unit;
}

// La pagina pubblica di una piattaforma, come la costruisce Windows (LiveLinkView.UrlOf): dove
// porta il marchio della riga. Vuota per le piattaforme senza un indirizzo noto.
QString liveUrlFor(const QString& known, const QString& channelName) {
    const QString name = channelName.trimmed();
    if (name.isEmpty())
        return {};
    const auto escaped = [](const QString& text) {
        return QString::fromUtf8(QUrl::toPercentEncoding(text).constData());
    };
    const auto withoutAt = [](const QString& text) {
        return text.startsWith('@') ? text.mid(1) : text;
    };
    if (known == "twitch")
        return "https://www.twitch.tv/" + escaped(name);
    if (known == "youtube") {
        // Un canale per id si indirizza con il suo; un handle non è un indirizzo che Studio
        // accetta, quindi si apre quello dell'account che sta trasmettendo.
        const bool channelId = name.startsWith("UC") && name.size() == 24;
        return "https://studio.youtube.com/channel/" + (channelId ? escaped(name) : QString("UC")) +
               "/livestreaming";
    }
    if (known == "kick")
        return "https://kick.com/" + escaped(name.toLower());
    if (known == "facebook")
        return "https://www.facebook.com/" + escaped(withoutAt(name));
    if (known == "tiktok")
        return "https://www.tiktok.com/@" + escaped(withoutAt(name)) + "/live";
    return {};
}

// L'etichetta della piattaforma, come la mostra anche il suo marchio.
QString platformLabel(const QString& known) {
    if (known == "youtube")
        return "YouTube";
    if (known == "tiktok")
        return "TikTok";
    return known.left(1).toUpper() + known.mid(1);
}

class GaugeWidget : public QWidget {
    double m_percentage = -1;

  public:
    explicit GaugeWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(100, 50);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setPercentage(double p) {
        m_percentage = p;
        update();
    }

    QSize sizeHint() const override {
        return {100, 50};
    }

  protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const bool light = qApp->property("orbisTheme").toString() == "light";
        QColor bgColor = light ? QColor(0, 0, 0, 20) : QColor(255, 255, 255, 20);
        QColor fgColor = palette().color(QPalette::Highlight);

        double penWidth = 8;
        double size = std::min(width() * 1.0, height() * 2.0) - penWidth;
        QRectF arcRect((width() - size) / 2.0, height() - size / 2.0 - penWidth / 2.0, size, size);

        QPen bgPen(bgColor, penWidth, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(bgPen);
        painter.drawArc(arcRect, 180 * 16, -180 * 16);

        if (m_percentage >= 0) {
            QPen fgPen(fgColor, penWidth, Qt::SolidLine, Qt::RoundCap);
            painter.setPen(fgPen);
            painter.drawArc(arcRect, 180 * 16, -180 * 16 * (m_percentage / 100.0));
        }
    }
};

} // namespace

// La finestra richiama il motore direttamente: nessun server HTTP, browser o WebView è avviato.
NativeWindow::NativeWindow(Studio& studio, fs::path resources)
    : studio(studio), resources(std::move(resources)),
      translations(QString::fromStdString((this->resources / "ui").string())),
      proxies(studio.store.root / "proxies") {
    QSettings appearance(QString::fromStdString((studio.store.root / "appearance.ini").string()),
                         QSettings::IniFormat);
    applyNativeTheme(appearance.value("theme", "dark").toString());
    setObjectName("nativeWindow");
    setWindowTitle(QString("OrbisStream · %1").arg(ORBIS_VERSION));
    resize(1380, 900);
    setMinimumSize(640, 640);
    setWindowIcon(QIcon(QString::fromStdString((this->resources / "app-icon.png").string())));
    auto preferences = studio.store.list("preferences");
    QString saved;
    for (size_t i = 0; i < preferences.size(); ++i)
        if (preferences.at(i).get("id").num() == 1)
            saved = s(preferences.at(i).get("language"));
    const auto system = QLocale::system().name().section('_', 0, 0);
    translations.setLanguage(translations.languages().contains(saved)    ? saved
                             : translations.languages().contains(system) ? system
                                                                         : "en");
    qApp->installTranslator(&translations);
    auto central = new QWidget(this);
    auto horizontal = new QHBoxLayout(central);
    horizontal->setContentsMargins(12, 12, 12, 12);
    horizontal->setSpacing(12);
    auto sidebar = new QWidget(central);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(228);
    sidebarClosed = appearance.value("sidebarClosed", false).toBool();
    auto side = new QVBoxLayout(sidebar);
    side->setContentsMargins(8, 14, 8, 14);
    side->setSpacing(3);
    auto brandRow = new QWidget(sidebar);
    brandRow->setObjectName("sidebarTitle");
    auto brandLayout = new QHBoxLayout(brandRow);
    brandLayout->setContentsMargins(8, 4, 8, 16);
    brandLayout->setSpacing(10);
    auto toggle = new QToolButton(brandRow);
    toggle->setObjectName("sidebarToggle");
    toggle->setIcon(studioIcon("menu"));
    toggle->setFixedSize(24, 28);
    toggle->setToolTip("Toggle Sidebar");
    toggle->setAccessibleName("Toggle Sidebar");
    toggle->setCheckable(true);
    toggle->setChecked(!sidebarClosed);
    connect(toggle, &QToolButton::clicked, this, [this] {
        sidebarClosed = !sidebarClosed;
        QSettings appearance(
            QString::fromStdString((this->studio.store.root / "appearance.ini").string()),
            QSettings::IniFormat);
        appearance.setValue("sidebarClosed", sidebarClosed);
        updateSidebar();
    });
    brandLayout->addWidget(toggle);
    auto mark = new QLabel(brandRow);
    mark->setObjectName("sidebarLogo");
    mark->setPixmap(
        QIcon(QString::fromStdString((this->resources / "favicon.png").string())).pixmap(20, 20));
    mark->setAlignment(Qt::AlignCenter);
    mark->setFixedSize(28, 28);
    brandLayout->addWidget(mark);
    auto brand = new QLabel("Orbis Stream", brandRow);
    brand->setObjectName("sidebarBrand");
    brandLayout->addWidget(brand, 1);
    side->addWidget(brandRow);
    navigation = new QListWidget(sidebar);
    navigation->setObjectName("navigation");
    navigation->setIconSize(QSize(15, 15));
    navigation->setSpacing(0);
    navigation->setFixedHeight(7 * 35);
    navigation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigationKeys = {"dashboard",   "live",   "livePreviewTitle", "layouts",
                      "taskManager", "manual", "settings"};
    navigationPages = {"dashboard", "liveMenu", "preview", "layouts",
                       "activity",  "manual",   "settings"};
    side->addWidget(navigation);
    side->addStretch();
    auto themes = new QWidget(sidebar);
    themes->setObjectName("themeSelector");
    auto themeRow = new QHBoxLayout(themes);
    themeRow->setContentsMargins(3, 3, 3, 3);
    themeRow->setSpacing(2);
    for (const auto& theme : {QString("light"), QString("dark")}) {
        auto choice = new QToolButton(themes);
        choice->setObjectName("theme_" + theme);
        choice->setProperty("themeChoice", true);
        choice->setFixedHeight(24);
        choice->setCheckable(true);
        choice->setChecked(qApp->property("orbisTheme").toString() == theme);
        choice->setIcon(studioIcon(theme));
        choice->setToolTip(theme == "light" ? "Light Theme" : "Dark Theme");
        choice->setAccessibleName(choice->toolTip());
        choice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        themeRow->addWidget(choice);
        connect(choice, &QToolButton::clicked, this, [this, theme] { setTheme(theme); });
    }
    side->addWidget(themes);
    languagePicker = new SidebarLanguagePicker(sidebar);
    languagePicker->setObjectName("languagePicker");
    const QMap<QString, QString> flags = {{"en", "gb"}, {"zh", "cn"}, {"el", "gr"}, {"ja", "jp"},
                                          {"ko", "kr"}, {"hi", "in"}, {"da", "dk"}, {"fil", "ph"}};
    languagePicker->setIconSize(QSize(22, 15));
    for (const auto& language : translations.languages()) {
        auto file = flags.value(language, language) + ".png";
        languagePicker->addItem(
            QIcon(QString::fromStdString((this->resources / "flags").string()) + "/" + file),
            NativeTranslations::languageName(language), language);
    }
    languagePicker->setCurrentIndex(languagePicker->findData(translations.language()));
    languagePicker->setAccessibleName("Language");
    languagePicker->setFixedHeight(30);
    side->addWidget(languagePicker);
    auto about = new QPushButton(sidebar);
    about->setObjectName("sidebarAbout");
    about->setCursor(Qt::PointingHandCursor);
    about->setAccessibleName(t("about"));
    about->setToolTip(t("about"));
    auto aboutRow = new QHBoxLayout(about);
    aboutRow->setContentsMargins(10, 10, 10, 6);
    aboutRow->setSpacing(10);
    auto info = new QLabel(about);
    info->setObjectName("sidebarInfoIcon");
    info->setPixmap(studioIcon("info").pixmap(15, 15));
    aboutRow->addWidget(info);
    auto aboutText = new QVBoxLayout;
    aboutText->setSpacing(2);
    auto version = new QLabel(QString("Orbis Stream %1").arg(ORBIS_VERSION), about);
    version->setObjectName("sidebarVersion");
    aboutText->addWidget(version);
    auto author = new QLabel(about);
    author->setObjectName("sidebarAuthor");
    author->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    aboutText->addWidget(author);
    aboutRow->addLayout(aboutText, 1);
    for (auto label : {info, version, author})
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    side->addWidget(about);
    connect(about, &QPushButton::clicked, this, &NativeWindow::showAbout);
    horizontal->addWidget(sidebar);
    auto scroll = new QScrollArea(central);
    scroll->setWidgetResizable(true);
    body = new QWidget(scroll);
    body->setObjectName("workspace");
    bodyLayout = new QVBoxLayout(body);
    bodyLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    bodyLayout->setContentsMargins(32, 26, 32, 26);
    bodyLayout->setSpacing(18);
    scroll->setWidget(body);
    horizontal->addWidget(scroll, 1);
    setCentralWidget(central);
    auto openNavigation = [this](QListWidgetItem* item) {
        const int row = navigation->row(item);
        if (row >= 0 && row < navigationPages.size())
            navigate(navigationPages.at(row));
    };
    connect(navigation, &QListWidget::itemClicked, this, openNavigation);
    connect(navigation, &QListWidget::itemActivated, this, openNavigation);
    connect(languagePicker, &QComboBox::currentIndexChanged, this,
            [this] { run([this] { setLanguage(languagePicker->currentData().toString()); }); });
    auto timer = new QTimer(this);
    timer->setInterval(1500);
    connect(timer, &QTimer::timeout, this, &NativeWindow::sample);
    timer->start();
    auto frames = new QTimer(this);
    frames->setInterval(200);
    connect(frames, &QTimer::timeout, this, &NativeWindow::updatePreview);
    frames->start();
    rebuild();
    sample();
}

// I lavori Qt devono terminare prima che vengano distrutti Studio e SQLite.
NativeWindow::~NativeWindow() {
    QThreadPool::globalInstance()->waitForDone();
    // Un lavoro già avviato può creare una sessione: fermarla dopo aver atteso i worker.
    studio.stopAll();
    qApp->removeTranslator(&translations);
}

void NativeWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if (navigation && languagePicker && body)
        updateSidebar();
}

// Stesse modalità della shell Windows: aperta, stretta sotto 800px, oppure chiusa
// esplicitamente. La chiusura manuale resta memorizzata anche al cambio di pagina.
void NativeWindow::updateSidebar() {
    auto sidebar = findChild<QWidget*>("sidebar");
    const bool compact = width() <= 800;
    const bool small = compact || sidebarClosed;
    sidebar->setProperty("closed", sidebarClosed);
    sidebar->setProperty("compact", compact);
    sidebar->setFixedWidth(sidebarClosed ? 34 : compact ? 50 : 228);
    sidebar->layout()->setContentsMargins(sidebarClosed ? 0
                                          : compact     ? 4
                                                        : 8,
                                          compact ? 10 : 14,
                                          sidebarClosed ? 0
                                          : compact     ? 4
                                                        : 8,
                                          14);
    auto title = findChild<QWidget*>("sidebarTitle");
    auto titleLayout = qobject_cast<QBoxLayout*>(title->layout());
    titleLayout->setDirection(small ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    titleLayout->setContentsMargins(small ? 0 : 8, small ? 0 : 4, small ? 0 : 8,
                                    sidebarClosed ? 0
                                    : compact     ? 12
                                                  : 16);
    findChild<QLabel*>("sidebarBrand")->setVisible(!small);
    findChild<QLabel*>("sidebarLogo")->setVisible(!sidebarClosed);
    title->setFixedHeight(sidebarClosed ? 28 : compact ? 78 : 48);
    navigation->setVisible(!sidebarClosed);
    for (int i = 0; i < navigation->count(); ++i) {
        auto item = navigation->item(i);
        item->setText(compact ? QString{} : item->toolTip());
    }
    findChild<QWidget*>("themeSelector")->setVisible(!small);
    languagePicker->setVisible(!sidebarClosed);
    languagePicker->setToolTip(NativeTranslations::languageName(translations.language()));
    languagePicker->setProperty("compact", compact);
    auto about = findChild<QPushButton*>("sidebarAbout");
    about->setFixedHeight(small ? 32 : 50);
    about->layout()->setContentsMargins(small ? 6 : 10, small ? 8 : 10, small ? 6 : 10, 6);
    about->setVisible(!sidebarClosed);
    about->setAccessibleName(t("about"));
    about->setToolTip(t("about"));
    auto author = findChild<QLabel*>("sidebarAuthor");
    author->setText(t("aboutAuthor") + " Marco Amleto Guarino");
    author->setToolTip(author->text());
    author->setVisible(!small);
    findChild<QLabel*>("sidebarVersion")->setVisible(!small);
    findChild<QLabel*>("sidebarInfoIcon")->setPixmap(studioIcon("info").pixmap(15, 15));
    auto toggle = findChild<QToolButton*>("sidebarToggle");
    toggle->setIcon(studioIcon("menu"));
    toggle->setChecked(!sidebarClosed);
    toggle->setProperty("expanded", !sidebarClosed);
    auto outer = qobject_cast<QHBoxLayout*>(centralWidget()->layout());
    outer->setContentsMargins(compact ? 6 : 12, compact ? 6 : 12, compact ? 6 : 12,
                              compact ? 6 : 12);
    outer->setSpacing(compact ? 6 : 12);
    for (auto widget : sidebar->findChildren<QWidget*>()) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
    sidebar->style()->unpolish(sidebar);
    sidebar->style()->polish(sidebar);
    sidebar->update();
}

void NativeWindow::showAbout() {
    QDialog dialog(this);
    dialog.setObjectName("aboutDialog");
    dialog.setWindowTitle(t("about"));
    auto column = new QVBoxLayout(&dialog);
    column->setContentsMargins(28, 24, 28, 24);
    column->setSpacing(16);
    auto logo = new QLabel(&dialog);
    logo->setPixmap(
        QIcon(QString::fromStdString((resources / "favicon.png").string())).pixmap(72, 72));
    column->addWidget(logo, 0, Qt::AlignHCenter);
    auto name = new QLabel("Orbis Stream", &dialog);
    name->setObjectName("cardTitle");
    column->addWidget(name, 0, Qt::AlignHCenter);
    column->addWidget(new QLabel(t("aboutVersion") + "  " + ORBIS_VERSION, &dialog));
    column->addWidget(new QLabel(t("aboutAuthor") + "  Marco Amleto Guarino", &dialog));
    auto linkedin = new QPushButton(t("aboutLinkedIn"), &dialog);
    linkedin->setObjectName("aboutLinkedIn");
    connect(linkedin, &QPushButton::clicked, &dialog, [] {
        QDesktopServices::openUrl(QUrl("https://www.linkedin.com/in/marco-amleto-guarino/"));
    });
    column->addWidget(linkedin);
    auto close = new QPushButton(t("close"), &dialog);
    close->setObjectName("closeAbout");
    close->setProperty("primary", true);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
    column->addWidget(close, 0, Qt::AlignRight);
    dialog.exec();
}

void NativeWindow::buildLiveMenu() {
    addDashboardCards({{"liveManagement", "live", "liveManagementDescription", "video"},
                       {"preview", "preview", "previewDescription", "screen"},
                       {"layouts", "layouts", "layoutsDescription", "layouts"},
                       {"liveHistory", "history", "liveHistoryDescription", "history"}});
}

// La chiusura dalla finestra chiede conferma se interromperebbe una trasmissione.
void NativeWindow::closeEvent(QCloseEvent* event) {
    const auto lives = studio.live();
    bool running = false;
    for (size_t i = 0; i < lives.size(); ++i) {
        const auto status = lives.at(i).get("status").str();
        if (status == "LIVE" || status == "STARTING" || status == "RECONNECTING")
            running = true;
    }
    if (running && !confirm("stopLive"))
        event->ignore();
    else {
        event->accept();
        for (auto timer : findChildren<QTimer*>())
            timer->stop();
        // La finestra principale governa la vita dell'app anche con player o dialoghi aperti.
        QCoreApplication::quit();
    }
}

QString NativeWindow::t(const char* key) const {
    return translations.text(key);
}

// Il cambio lingua viene salvato nei dati dell'app e ridisegna subito navigazione e schermata
// corrente.
void NativeWindow::setLanguage(const QString& language) {
    translations.setLanguage(language);
    auto all = studio.store.list("preferences");
    Json preferences;
    for (size_t i = 0; i < all.size(); ++i)
        if (all.at(i).get("id").num() == 1)
            preferences = all.at(i).clone();
    preferences.set("language", language.toStdString());
    studio.store.save("preferences", preferences, 1);
    languagePicker->blockSignals(true);
    languagePicker->setCurrentIndex(languagePicker->findData(language));
    languagePicker->blockSignals(false);
    rebuild();
}

// Le identità delle pagine sono stabili: cambiare lingua non cambia la navigazione né la selezione
// dati.
void NativeWindow::navigate(const QString& key) {
    page = key;
    currentPage = 0;
    rebuild();
}

// Ricostruisce solo quando cambia pagina o lingua; il campionamento periodico aggiorna le righe
// esistenti.
void NativeWindow::setTheme(const QString& theme) {
    if (theme != "light" && theme != "dark")
        return;
    QSettings appearance(QString::fromStdString((studio.store.root / "appearance.ini").string()),
                         QSettings::IniFormat);
    appearance.setValue("theme", theme);
    applyNativeTheme(theme);
    for (const auto& name : {QString("light"), QString("dark")}) {
        auto choice = findChild<QToolButton*>("theme_" + name);
        choice->setChecked(theme == name);
        choice->setIcon(studioIcon(name));
    }
    rebuild();
}

void NativeWindow::rebuild() {
    bodyLayout->setContentsMargins(14, 2, 14, 36);
    bodyLayout->setSpacing(page == "dashboard" ? 0 : 12);
    body->setProperty("dashboard", page == "dashboard");
    if (cleanupLabel) delete cleanupLabel->parentWidget();
    table = nullptr;
    configurationSignature.clear();
    search = nullptr;
    channelFilter = statusFilter = platformFilter = livePicker = nullptr;
    preview = previewFacts = cleanupLabel = pageLabel = emptyState = nullptr;
    cleanupBar = nullptr;
    previewContent = previewMixer = previewDeck = nullptr;
    previewControlsFor = 0;
    previewParameterTimer = nullptr;
    while (auto item = bodyLayout->takeAt(0)) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
    navigation->blockSignals(true);
    navigation->clear();
    for (int i = 0; i < navigationKeys.size(); ++i) {
        const auto label = navigationPages.at(i) == "liveMenu"
                               ? QString("Live")
                               : t(navigationKeys.at(i).toUtf8().constData());
        const QStringList glyphs = {"home",     "video",  "screen",  "layouts",
                                    "activity", "manual", "settings"};
        auto icon = studioIcon(glyphs.at(i));
        icon.addPixmap(studioIcon(glyphs.at(i), Qt::white).pixmap(15, 15), QIcon::Selected);
        auto item = new QListWidgetItem(icon, label, navigation);
        item->setToolTip(label);
        item->setData(Qt::AccessibleTextRole, label);
        const QStringList paths = {"/orbis/mainMenu",        "/orbis/mainLiveMenu",
                                   "/orbis/mainPreview",     "/orbis/mainLayoutList",
                                   "/orbis/mainTaskManager", "/orbis/manual",
                                   "/orbis/mainSetting"};
        item->setData(Qt::UserRole, paths.at(i));
        item->setSizeHint(QSize(0, 35));
    }
    int selected = navigationPages.indexOf(page);
    if (selected < 0)
        selected = navigationPages.indexOf(page == "scenes" || page == "overlays" ? "layouts"
                                           : page == "live" || page == "playlist" ||
                                                   page == "buttons" || page == "history"
                                               ? "liveMenu"
                                               : "settings");
    navigation->setCurrentRow(selected);
    navigation->blockSignals(false);
    updateSidebar();
    const QMap<QString, const char*> titles = {
        {"dashboard", "dashboard"},    {"live", "liveManagement"},
        {"history", "liveHistory"},    {"preview", "livePreviewTitle"},
        {"layouts", "layouts"},        {"scenes", "sceneStrip"},
        {"activity", "taskManager"},   {"manual", "manual"},
        {"settings", "settings"},      {"channels", "channelSettings"},
        {"profiles", "videoSettings"}, {"playlist", "nativePlaylistLibrary"},
        {"overlays", "overlays"},      {"buttons", "sceneDeck"},
        {"scene-media", "sceneMedia"}};
    auto title = new QLabel(page == "liveMenu" ? "Live" : t(titles.value(page, "dashboard")), body);
    title->setObjectName("pageTitle");
    auto heading = new QFrame(body);
    heading->setObjectName("pageHeading");
    auto headingLayout = new QHBoxLayout(heading);
    headingLayout->setContentsMargins(20, 18, 20, 18);
    auto headingText = new QVBoxLayout;
    headingText->addWidget(title);
    const QMap<QString, const char*> descriptions = {
        {"dashboard", "dashboardDescription"},
        {"live", "liveManagementDescription"},
        {"liveMenu", "liveManagementDescription"},
        {"history", "liveHistoryDescription"},
        {"preview", "livePreviewSubtitle"},
        {"layouts", "layoutsDescription"},
        {"scenes", "canvasComposeHint"},
        {"channels", "channelSettingsDescription"},
        {"profiles", "additionalVideoOptionsDescription"},
        {"settings", "selectSectionMessage"},
        {"activity", "taskManagerDescription"},
        {"buttons", "sceneDeckHint"},
        {"overlays", "overlaysHint"}};
    if (descriptions.contains(page)) {
        auto description = new QLabel(t(descriptions.value(page)), heading);
        description->setObjectName("muted");
        description->setWordWrap(true);
        headingText->addWidget(description);
    }
    headingLayout->addLayout(headingText, 1);
    if (page == "live" || page == "liveMenu") {
        auto actions = new QWidget(heading);
        auto row = new QHBoxLayout(actions);
        row->setContentsMargins(12, 0, 0, 0);
        // Come la pagina Windows: sblocca (solo con video bloccati), avvia playlist, avvia live.
        if (page == "live") {
            const auto videos = studio.store.list("videos");
            for (size_t i = 0; i < videos.size(); ++i)
                if (videos.at(i).get("isLocked").flag()) {
                    addAction("unLockVideo", [this] { unlockVideos(); }, actions, "unlock");
                    break;
                }
        }
        if (page != "liveMenu")
            addAction("startPlaylistButton", [this] { startWizard(true); }, actions, "playlist");
        addAction("startLiveButton", [this] { startWizard(); }, actions, "play");
        headingLayout->addWidget(actions);
    }
    if (page == "dashboard") {
        headingLayout->setContentsMargins(20, 12, 20, 12);
        headingLayout->setSpacing(14);
        headingText->setSpacing(3);
        title->setProperty("dashboardTitle", true);
        auto coffee = new QWidget(heading);
        auto column = new QVBoxLayout(coffee);
        column->setContentsMargins(0, 0, 0, 0);
        column->setSpacing(6);
        auto donate = new CoffeeButton(
            QString::fromUtf8("🦦  ") + t("payMeAWeasel") + QString::fromUtf8("  ☕"), coffee);
        donate->setAccessibleName(t("payMeAWeasel"));
        donate->setProperty("url", "https://buymeacoffee.com/zaccalos");
        connect(donate, &QPushButton::clicked, this,
                [] { QDesktopServices::openUrl(QUrl("https://buymeacoffee.com/zaccalos")); });
        column->addWidget(donate, 0, Qt::AlignRight);
        auto note = new QLabel(t("coffeeForPerla"), coffee);
        note->setObjectName("coffeePerla");
        note->setWordWrap(true);
        note->setAlignment(Qt::AlignRight);
        column->addWidget(note);
        coffee->setMaximumWidth(300);
        headingLayout->addWidget(coffee);
    }
    if (page == "liveMenu" || page == "settings" || page == "live" || page == "history" ||
        page == "preview") {
        auto back = new QToolButton(heading);
        back->setObjectName("menuBack");
        back->setProperty("rowAction", true);
        back->setIcon(studioIcon("back"));
        back->setToolTip(t("back"));
        back->setAccessibleName(t("back"));
        connect(back, &QToolButton::clicked, this, [this] {
            navigate(page == "liveMenu" || page == "settings" ? "dashboard" : "liveMenu");
        });
        headingLayout->insertWidget(0, back);
    }
    bodyLayout->addWidget(heading);
    if (page == "dashboard")
        buildDashboard();
    else if (page == "liveMenu")
        buildLiveMenu();
    else if (page == "channels")
        buildChannels();
    else if (page == "profiles")
        buildProfiles();
    else if (page == "live" || page == "history")
        buildLive(page == "history");
    else if (page == "layouts" || page == "scenes")
        buildScenes(page == "layouts");
    else if (page == "preview")
        buildPreview();
    else if (page == "playlist")
        buildPlaylist();
    else if (page == "buttons")
        buildButtons();
    else if (page == "overlays" || page == "scene-media")
        buildMedia(page);
    else if (page == "activity")
        buildMetrics();
    else if (page == "manual")
        buildManual();
    else
        buildSettings();
    bodyLayout->addStretch();
    refreshRows();
}

// I/O, FFprobe e codifica non bloccano il ciclo eventi: risultato ed errori tornano nel thread
// grafico.
void NativeWindow::runAsync(std::function<Json()> job, std::function<void(Json)> finished) {
    auto watcher = new QFutureWatcher<JobResult>(this);
    statusBar()->showMessage(t("loading"));
    connect(watcher, &QFutureWatcher<JobResult>::finished, this, [this, watcher, finished] {
        auto result = watcher->result();
        watcher->deleteLater();
        if (!result.error.empty()) {
            std::runtime_error error(result.error);
            showError(error);
        } else {
            if (finished)
                run([&] { finished(result.value); });
            statusBar()->showMessage(t("addedSuccessfully"), 3000);
        }
        sample();
    });
    watcher->setFuture(QtConcurrent::run([job] {
        try {
            return JobResult{job(), {}};
        } catch (const std::exception& e) {
            return JobResult{Json{}, e.what()};
        }
    }));
}

// Le operazioni SQLite brevi condividono la stessa gestione degli errori dei lavori asincroni.
// Raccoglie gli errori delle azioni brevi nel thread Qt e aggiorna lo stato solo dopo l’operazione.
void NativeWindow::run(const std::function<void()>& action) {
    try {
        action();
        sample();
    } catch (const std::exception& error) {
        showError(error);
    }
}

// Le notifiche sono localizzate; i dettagli tecnici restano nei log invece di diventare etichette
// italiane fisse.
// Mostra un messaggio tradotto e separa la diagnostica tecnica, utile quando il motore rifiuta una
// richiesta.
void NativeWindow::showError(const std::exception& error) {
    const std::string message = error.what();
    const char* key = message.find("Nome scena") != std::string::npos ? "sceneNeedsName"
                      : message.find("almeno una sorgente video") != std::string::npos
                          ? "sceneNeedsPicture"
                      : message.find("Chiave richiesta") != std::string::npos ? "tiktokKeyHint"
                                                                              : "invalidParameter";
    QMessageBox box(QMessageBox::Warning, t("error"), t(key), QMessageBox::Ok, this);
    box.setDetailedText(QString::fromUtf8(error.what()));
    box.setObjectName("errorDialog");
    box.exec();
}

// Usa la stessa chiave di conferma in ogni lingua, sostituendo il nome senza modificarne il
// catalogo.
bool NativeWindow::confirm(const char* key, const QString& subject) {
    return QMessageBox::question(
               this, t("confirm"),
               t(key).replace("{0}", subject.isEmpty() ? t("configuration") : subject),
               QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}

// Crea controlli Qt, con chiave stabile anche nei test in lingue diverse.
void NativeWindow::addAction(const char* key, const std::function<void()>& action, QWidget* parent,
                             const QString& icon) {
    auto button = new QPushButton(t(key), parent ? parent : body);
    button->setObjectName(key);
    button->setAccessibleName(button->text());
    button->setCursor(Qt::PointingHandCursor);
    const QString name(key);
    button->setProperty("primary", name.startsWith("add") || name == "startLiveButton" ||
                                       name == "newLayout" || name == "composeScene");
    button->setProperty("danger", name.contains("stop", Qt::CaseInsensitive) || name == "delete");
    if (!icon.isEmpty())
        button->setIcon(studioIcon(icon));
    else if (name.startsWith("add") || name == "newLayout")
        button->setIcon(studioIcon("plus"));
    else if (name.startsWith("start"))
        button->setIcon(studioIcon("play"));
    connect(button, &QPushButton::clicked, this, [this, action] { run(action); });
    if (parent && parent->layout())
        parent->layout()->addWidget(button);
    else {
        // Le azioni contigue condividono una griglia, evitando colonne di pulsanti a tutta
        // larghezza.
        QWidget* group = nullptr;
        if (bodyLayout->count())
            group = bodyLayout->itemAt(bodyLayout->count() - 1)->widget();
        if (!group || group->objectName() != "actionGroup") {
            group = new QWidget(body);
            group->setObjectName("actionGroup");
            auto grid = new QGridLayout(group);
            grid->setContentsMargins(0, 0, 0, 0);
            grid->setSpacing(10);
            bodyLayout->addWidget(group);
        }
        auto grid = qobject_cast<QGridLayout*>(group->layout());
        const int index = grid->count();
        grid->addWidget(button, index / 3, index % 3);
    }
}

// Tabelle di sola lettura: le modifiche passano dai dialoghi validati, evitando dati parziali nelle
// celle.
QTableWidget* NativeWindow::makeTable(const QStringList& headers) {
    auto result = new QTableWidget(0, headers.size(), body);
    result->setObjectName("recordsTable");
    result->setHorizontalHeaderLabels(headers);
    result->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    result->setSelectionBehavior(QAbstractItemView::SelectRows);
    result->setSelectionMode(QAbstractItemView::SingleSelection);
    result->setEditTriggers(QAbstractItemView::NoEditTriggers);
    result->setAlternatingRowColors(true);
    result->setShowGrid(false);
    result->verticalHeader()->hide();
    result->verticalHeader()->setDefaultSectionSize(54);
    result->horizontalHeader()->setMinimumSectionSize(90);
    result->setWordWrap(false);
    result->setMinimumHeight(380);
    bodyLayout->addWidget(result);
    emptyState = new QLabel(t("noDataPresent"), body);
    emptyState->setObjectName("emptyState");
    emptyState->setAlignment(Qt::AlignCenter);
    emptyState->setWordWrap(true);
    emptyState->setMinimumHeight(180);
    bodyLayout->addWidget(emptyState);
    return result;
}

// Le azioni di pagina restano fuori dalla tabella e i filtri non si perdono durante il polling.
void NativeWindow::addPager(int size) {
    pageSize = size;
    auto row = new QWidget(body);
    auto layout = new QHBoxLayout(row);
    auto previous = new QPushButton(t("back"), row), next = new QPushButton(t("next"), row);
    previous->setObjectName("previousPage");
    next->setObjectName("nextPage");
    pageLabel = new QLabel(row);
    if (page == "live" || page == "history") {
        layout->addStretch();
        previous->setText({});
        previous->setIcon(studioIcon("back"));
        previous->setAccessibleName(t("back"));
        previous->setFixedWidth(32);
        next->setText({});
        next->setIcon(studioIcon("next"));
        next->setAccessibleName(t("next"));
        next->setFixedWidth(32);
        pageLabel->hide();
    }
    layout->addWidget(previous);
    layout->addWidget(pageLabel);
    if (page == "live" || page == "history") {
        auto numbers = new QWidget(row);
        numbers->setObjectName("livePageNumbers");
        auto numberRow = new QHBoxLayout(numbers);
        numberRow->setContentsMargins(0, 0, 0, 0);
        numberRow->setSpacing(4);
        for (int i = 0; i < 5; ++i) {
            auto button = new QPushButton(numbers);
            button->setCheckable(true);
            button->setFixedWidth(32);
            button->setObjectName(QString("livePage%1").arg(i));
            numberRow->addWidget(button);
            connect(button, &QPushButton::clicked, this, [this, button] {
                currentPage = button->property("page").toInt();
                refreshRows();
            });
        }
        layout->addWidget(numbers);
    }
    layout->addWidget(next);
    if (page == "live" || page == "history")
        layout->addStretch();
    bodyLayout->addWidget(row);
    connect(previous, &QPushButton::clicked, this, [this] {
        currentPage = std::max(0, currentPage - 1);
        refreshRows();
    });
    connect(next, &QPushButton::clicked, this, [this] {
        ++currentPage;
        refreshRows();
    });
}

// Stato e metriche sono campionati in un worker, senza aprire un socket o dipendere dal browser.
void NativeWindow::sample() {
    if (sampling)
        return;
    sampling = true;
    auto watcher = new QFutureWatcher<JobResult>(this);
    connect(watcher, &QFutureWatcher<JobResult>::finished, this, [this, watcher] {
        const auto result = watcher->result();
        watcher->deleteLater();
        sampling = false;
        if (result.error.empty()) {
            state = result.value;
            refreshRows();
        } else
            statusBar()->showMessage(t("error"));
    });
    watcher->setFuture(QtConcurrent::run([this] {
        try {
            Json result;
            for (auto kind : {"channels", "profiles", "scenes", "videos", "buttons", "history"})
                result.set(kind, studio.store.list(kind));
            auto live = studio.live();
            result.set("live", live);
            result.set("liveRows", liveRows(studio.store, live));
            result.set("metrics", metrics(studio.store.root));
            result.set("process", processMetrics(getpid()));
            result.set("cleanup", studio.historyCleanup->progress());
            return JobResult{result, {}};
        } catch (const std::exception& e) {
            return JobResult{Json{}, e.what()};
        }
    }));
}

// Le schede seguono le sezioni del manuale: titolo e spiegazione sono un unico bersaglio
// cliccabile.
void NativeWindow::addDestinationCards(const QList<QStringList>& entries) {
    addDashboardCards(entries);
}

// Stesse sezioni, destinazioni e ordine di Pages/MainMenu.cshtml.
void NativeWindow::buildDashboard() {
    auto section = [this](const char* key, const QList<QStringList>& entries) {
        bodyLayout->addSpacing(22);
        auto heading = new QWidget(body);
        auto row = new QHBoxLayout(heading);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(8);
        auto accent = new QLabel(heading);
        accent->setObjectName("dashboardSectionAccent");
        accent->setFixedSize(3, 12);
        row->addWidget(accent);
        auto title = new QLabel(t(key), heading);
        title->setObjectName("dashboardSectionTitle");
        row->addWidget(title, 1);
        bodyLayout->addWidget(heading);
        bodyLayout->addSpacing(10);
        addDashboardCards(entries, key);
    };
    section("dashboardSectionLive",
            {{"liveManagement", "live", "liveManagementDescription", "video"},
             {"preview", "preview", "previewDescription", "screen"},
             {"layouts", "layouts", "layoutsDescription", "layouts"},
             {"liveHistory", "history", "liveHistoryDescription", "history"}});
    section("dashboardSectionSettings",
            {{"videoAndAudio", "profiles", "additionalVideoOptionsDescription", "video"},
             {"channelSettings", "channels", "channelSettingsDescription", "channelSettings"}});
    section("dashboardSectionSystem",
            {{"taskManager", "activity", "taskManagerDescription", "activity"}});
    section("manual", {{"manualTitle", "manual", "manualDescription", "manual"}});
}

// Filtri e ricerca restano fuori dalle righe per conservare il focus durante il campionamento.
// Layout e scene condividono l’editor ma restano distinti, perché solo i layout ammettono slot
// vuoti.
void NativeWindow::buildScenes(bool layouts) {
    QWidget* actions = nullptr;
    if (layouts) {
        auto heading = body->findChild<QFrame*>("pageHeading");
        auto row = qobject_cast<QHBoxLayout*>(heading->layout());
        row->setContentsMargins(20, 12, 20, 12);
        row->setSpacing(16);
        auto back = new QToolButton(heading);
        back->setObjectName("layoutBack");
        back->setProperty("rowAction", true);
        back->setIcon(studioIcon("back"));
        back->setFixedSize(32, 32);
        back->setToolTip(t("back"));
        back->setAccessibleName(t("back"));
        connect(back, &QToolButton::clicked, this, [this] { navigate("liveMenu"); });
        row->insertWidget(0, back);
        actions = new QWidget(heading);
        auto actionRow = new QHBoxLayout(actions);
        actionRow->setContentsMargins(0, 0, 0, 0);
        row->addWidget(actions);
    }
    addAction(
        layouts ? "newLayout" : "composeScene",
        [this, layouts] {
            Json scene;
            scene.set("name", t(layouts ? "newLayout" : "composeScene").toStdString());
            scene.set("width", 1280.0);
            scene.set("height", 720.0);
            scene.set("isLayout", layouts);
            scene.set("items", Json::array());
            editScene(scene);
        },
        actions);
    if (layouts) {
        auto list = new QWidget(body);
        list->setObjectName("layoutList");
        auto rows = new QVBoxLayout(list);
        rows->setContentsMargins(0, 0, 0, 0);
        rows->setSpacing(8);
        bodyLayout->addWidget(list);
        return;
    }
    addAction(layouts ? "sceneStrip" : "layouts",
              [this, layouts] { navigate(layouts ? "scenes" : "layouts"); });
    addAction("overlays", [this] { navigate("overlays"); });
    table = makeTable({t("title"), t("description"), t("resolution"), t("settings")});
}

// MainLayoutList: solo layout, con nome, descrizione e azioni visibili; nessuna tabella.
void NativeWindow::refreshLayouts() {
    auto list = body->findChild<QWidget*>("layoutList");
    if (!list)
        return;
    auto records = Json::array();
    auto scenes = state.get("scenes");
    for (size_t i = 0; i < scenes.size(); ++i)
        if (scenes.at(i).get("isLayout").flag())
            records.push(scenes.at(i));
    const auto signature = QString::fromStdString(records.dump());
    if (list->property("records").toString() == signature)
        return;
    list->setProperty("records", signature);
    auto rows = qobject_cast<QVBoxLayout*>(list->layout());
    while (auto item = rows->takeAt(0)) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->setParent(nullptr);
            item->widget()->deleteLater();
        }
        delete item;
    }
    if (!records.size()) {
        auto empty = new QFrame(list);
        empty->setObjectName("layoutEmpty");
        auto column = new QVBoxLayout(empty);
        column->setContentsMargins(16, 40, 16, 40);
        column->setSpacing(10);
        auto icon = new QLabel(empty);
        icon->setObjectName("layoutEmptyIcon");
        icon->setFixedSize(48, 48);
        icon->setAlignment(Qt::AlignCenter);
        icon->setPixmap(studioIcon("layouts").pixmap(22, 22));
        column->addWidget(icon, 0, Qt::AlignHCenter);
        auto label = new QLabel(t("noDataPresent"), empty);
        label->setObjectName("muted");
        label->setAlignment(Qt::AlignCenter);
        column->addWidget(label);
        rows->addWidget(empty);
    }
    for (size_t i = 0; i < records.size(); ++i) {
        auto record = records.at(i);
        auto card = new QFrame(list);
        card->setObjectName("layoutRow");
        auto row = new QHBoxLayout(card);
        row->setContentsMargins(16, 12, 16, 12);
        row->setSpacing(12);
        card->setMinimumHeight(64);
        auto icon = new QLabel(card);
        icon->setObjectName("layoutLead");
        icon->setFixedSize(36, 36);
        icon->setAlignment(Qt::AlignCenter);
        icon->setPixmap(studioIcon("layouts", QColor("#ffffff")).pixmap(18, 18));
        row->addWidget(icon);
        auto text = new QVBoxLayout;
        text->setSpacing(2);
        for (const auto& key : {"name", "description"}) {
            const auto value = s(record.get(key));
            if (value.trimmed().isEmpty())
                continue;
            auto label = new QLabel(value, card);
            label->setTextFormat(Qt::PlainText);
            label->setObjectName(QString(key) == "name" ? "rowTitle" : "muted");
            label->setToolTip(value);
            label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
            text->addWidget(label);
        }
        row->addLayout(text, 1);
        for (auto key : {"seeDetails", "delete"}) {
            const bool remove = QString(key) == "delete";
            auto action = new QToolButton(card);
            action->setObjectName(key);
            action->setProperty("rowAction", true);
            action->setIcon(
                studioIcon(remove ? "delete" : "edit", remove ? QColor("#ff453a") : QColor()));
            action->setFixedSize(32, 32);
            action->setToolTip(t(key));
            action->setAccessibleName(t(key));
            connect(action, &QToolButton::clicked, this, [this, record, remove] {
                run([&] {
                    if (!remove) {
                        editScene(record);
                    } else if (confirm("deleteLayoutConfirm")) {
                        const long id = record.get("id").num();
                        ensureSceneCanDelete(studio.store, id);
                        studio.store.remove("scenes", id);
                    }
                });
            });
            row->addWidget(action);
        }
        rows->addWidget(card);
    }
}

// Mantiene filtri e pulizia del flusso originale, con pagine da dieci dirette o venti storici.
// Ogni riga conserva l'ID nei dati Qt: ordinare o filtrare non cambia l'elemento su cui agisce un
// pulsante.
void NativeWindow::refreshRows() {
    // Non distrugge menu o selettori mentre l’utente sta scegliendo un’azione.
    if (QApplication::activePopupWidget())
        return;
    if (page == "channels" || page == "profiles") {
        refreshConfigurations();
        return;
    }
    if (page == "history") {
        refreshHistory();
        return;
    }
    if (page == "layouts") {
        refreshLayouts();
        return;
    }
    if (page == "dashboard") {
        return;
    }
    if (page == "preview") {
        updatePreview();
        return;
    }
    if (page == "activity") {
        buildMetrics();
        return;
    }
    if (!table)
        return;
    const QString query = search ? search->text() : "";
    QString kind = page == "channels"                      ? "channels"
                   : page == "profiles"                    ? "profiles"
                   : page == "layouts" || page == "scenes" ? "scenes"
                   : page == "playlist"                    ? "videos"
                   : page == "buttons"                     ? "buttons"
                                                           : "liveRows";
    auto records = state.get(kind.toStdString());
    if (channelFilter) {
        auto picked = channelFilter->currentData();
        QSet<QString> names;
        for (size_t i = 0; i < records.size(); ++i)
            if (!s(records.at(i).get("channelName")).isEmpty())
                names.insert(s(records.at(i).get("channelName")));
        channelFilter->blockSignals(true);
        channelFilter->clear();
        channelFilter->addItem("—", "");
        auto sorted = names.values();
        std::sort(sorted.begin(), sorted.end());
        for (const auto& name : sorted)
            channelFilter->addItem(name, name);
        channelFilter->setCurrentIndex(std::max(0, channelFilter->findData(picked)));
        channelFilter->blockSignals(false);
    }
    std::vector<Json> selected;
    for (size_t i = 0; i < records.size(); ++i) {
        auto record = records.at(i);
        if (page == "profiles" && record.get("isDefaultConfiguration").flag())
            continue;
        if ((page == "layouts" || page == "scenes") &&
            record.get("isLayout").flag() != (page == "layouts"))
            continue;
        if (!query.isEmpty() &&
            !QString::fromStdString(record.dump()).contains(query, Qt::CaseInsensitive))
            continue;
        if (platformFilter && !platformFilter->currentData().toString().isEmpty() &&
            s(record.get("platform")) != platformFilter->currentData().toString())
            continue;
        if (statusFilter && !statusFilter->currentData().toString().isEmpty() &&
            s(record.get("status")) != statusFilter->currentData().toString())
            continue;
        if (channelFilter && !channelFilter->currentData().toString().isEmpty() &&
            s(record.get("channelName")) != channelFilter->currentData().toString())
            continue;
        selected.push_back(record);
    }
    if (kind == "videos")
        std::stable_sort(selected.begin(), selected.end(), [](const Json& a, const Json& b) {
            return a.get("order").num(a.get("id").num()) < b.get("order").num(b.get("id").num());
        });
    if (pageLabel) {
        const int total = std::max(1, int((selected.size() + pageSize - 1) / pageSize));
        currentPage = std::clamp(currentPage, 0, total - 1);
        updatePager(total);
        pageLabel->parentWidget()->setVisible(!selected.empty());
        pageLabel->parentWidget()
            ->findChild<QPushButton*>("previousPage")
            ->setEnabled(currentPage > 0);
        pageLabel->parentWidget()
            ->findChild<QPushButton*>("nextPage")
            ->setEnabled(currentPage + 1 < total);
        pageLabel->setText(t("positionOf")
                               .replace("{0}", QLocale().toString(currentPage + 1))
                               .replace("{1}", QLocale().toString(total)));
        const auto first = std::min(selected.size(), size_t(currentPage * pageSize)),
                   last = std::min(selected.size(), first + pageSize);
        selected = std::vector<Json>(selected.begin() + first, selected.begin() + last);
    }
    if (emptyState) {
        emptyState->setVisible(selected.empty());
        table->setVisible(!selected.empty());
        emptyState->setText(t("noDataPresent") +
                            (page == "live" ? "\n\n" + t("startALiveHere") : QString{}));
    }
    const auto oldId = table->currentRow() >= 0
                           ? table->item(table->currentRow(), 0)->data(Qt::UserRole)
                           : QVariant{};
    table->setRowCount(0);
    liveActionsWidth = 0;
    for (const auto& record : selected) {
        const long id = record.get("id").num();
        const int row = table->rowCount();
        table->insertRow(row);
        // Gestione Live disegna ogni azione come icona visibile; le altre pagine conservano il
        // menu compatto.
        if (kind == "liveRows" && page == "live") {
            refreshLiveRow(row, record);
            if (oldId.toLongLong() == id)
                table->selectRow(row);
            continue;
        }
        QStringList values;
        if (kind == "channels")
            values = {s(record.get("channelName")), s(record.get("platform")),
                      s(record.get("streamUrl")),
                      t(record.get("isActive").flag(true) ? "active" : "deactive")};
        else if (kind == "profiles")
            values = {s(record.get("title")),
                      QString("%1×%2")
                          .arg(record.get("videoWidth").num())
                          .arg(record.get("videoHeight").num()),
                      QLocale().toString(record.get("frameRate").num()),
                      formatMetric(record.get("videoBitrate").num(), " kbps"),
                      t(record.get("isActive").flag() ? "active" : "deactive")};
        else if (kind == "scenes")
            values = {
                s(record.get("name")), s(record.get("description")),
                QString("%1×%2").arg(record.get("width").num()).arg(record.get("height").num())};
        else if (kind == "videos")
            values = {s(record.get("name")), s(record.get("videoPath")),
                      t(record.get("enabled").flag(true) ? "active" : "deactive")};
        else if (kind == "buttons")
            values = {s(record.get("label")), s(record.get("mediaName")), s(record.get("hotkey"))};
        else {
            const auto status = record.get("status").str();
            const char* key = status == "LIVE"       ? "liveStatusLive"
                              : status == "ENDED"    ? "liveStatusEnded"
                              : status == "STOPPED"  ? "liveStatusStopped"
                              : status == "ERROR"    ? "liveStatusError"
                              : status == "STARTING" ? "upcomingLive"
                                                     : "liveStatusOffline";
            values = {
                s(record.get("title")), t(key), s(record.get("channelName")),
                s(record.get("platform")),
                QLocale().toString(
                    QDateTime::fromString(s(record.get("startedAt")), Qt::ISODate).toLocalTime(),
                    QLocale::ShortFormat)};
        }
        for (int column = 0; column < values.size(); ++column) {
            auto item = new QTableWidgetItem(values[column]);
            item->setData(Qt::UserRole, qlonglong(id));
            table->setItem(row, column, item);
        }
        auto controls = new QWidget(table);
        auto layout = new QGridLayout(controls);
        layout->setContentsMargins(2, 2, 2, 2);
        // Un menu Qt mantiene tutte le azioni accessibili anche nelle colonne strette o in lingue
        // con testi lunghi.
        auto trigger = new QToolButton(controls);
        trigger->setText("•••");
        trigger->setAccessibleName(t("nativeActions"));
        trigger->setToolTip(t("nativeActions"));
        trigger->setPopupMode(QToolButton::InstantPopup);
        auto menu = new QMenu(trigger);
        trigger->setMenu(menu);
        auto action = [this, menu, layout, controls, running = record.get("running").flag()](
                          const char* key, std::function<void()> fn) {
            auto entry = menu->addAction(t(key));
            entry->setObjectName(key);
            connect(entry, &QAction::triggered, this, [this, fn] { run(fn); });
            const QString name(key);
            if (name == "seeDetails" || name == "sceneEdit" || name == "stopLiveIconLabel" ||
                name == "startLiveIconLabel" || name == "preview" || name == "sceneDeckOpen" ||
                (!running && (name == "playlistDetails" || name == "sceneDetails"))) {
                auto button = new QPushButton(
                    name == "seeDetails" ? t("nativeEdit") : entry->text(), controls);
                button->setObjectName("row_" + name);
                button->setProperty("compact", true);
                button->setProperty("danger", name == "stopLiveIconLabel");
                button->setProperty("primary", name == "startLiveIconLabel");
                button->setCursor(Qt::PointingHandCursor);
                connect(button, &QPushButton::clicked, entry, &QAction::trigger);
                const int index = layout->count();
                layout->addWidget(button, index / 2, index % 2);
            }
            return entry;
        };
        if (kind == "channels") {
            action("seeDetails", [this, record] { editChannel(record); });
            action(record.get("isActive").flag(true) ? "deactivateButton" : "activeButton",
                   [this, record, id] {
                       auto value = record.clone();
                       value.set("isActive", !value.get("isActive").flag(true));
                       studio.store.save("channels", value, id);
                   });
        } else if (kind == "profiles") {
            action("seeDetails", [this, record] { editProfile(record); });
            action("duplicate", [this, record] {
                auto copy = record.clone();
                copy.set("id", 0.0);
                copy.set("title",
                         record.get("title").str() + " · " + t("copySuffix").toStdString());
                editProfile(copy);
            });
            action(record.get("isActive").flag() ? "deactivateButton" : "activeButton",
                   [this, record, id] {
                       auto value = record.clone();
                       value.set("isActive", !value.get("isActive").flag());
                       studio.store.save("profiles", value, id);
                   });
        } else if (kind == "scenes") {
            action("seeDetails", [this, record] { editScene(record); });
            if (page == "layouts")
                action("composeScene", [this, record] {
                    auto copy = record.clone();
                    copy.set("id", 0.0);
                    copy.set("isLayout", false);
                    editScene(copy);
                });
        } else if (kind == "videos") {
            action("seeDetails", [this, record] { editVideo(record); });
            action("preview", [this, record] { playFile(s(record.get("videoPath"))); });
            action(record.get("enabled").flag(true) ? "deactivateButton" : "activeButton",
                   [this, record, id] {
                       auto value = record.clone();
                       value.set("enabled", !value.get("enabled").flag(true));
                       studio.store.save("videos", value, id);
                   });
        } else if (kind == "buttons")
            action("sceneEdit", [this, record] { editButton(record); });
        else {
            action(record.get("request").has("scene") ? "sceneDetails" : "playlistDetails",
                   [this, record] { showLiveDetails(record); });
            action("connectVideoSetting", [this, record] { linkProfile(record); });
            if (record.get("running").flag()) {
                action("stopLiveIconLabel", [this, id] {
                    if (confirm("stopLive"))
                        runAsync([this, id] {
                            studio.session(id)->stop();
                            return Json{};
                        });
                });
                action("preview", [this, id] {
                    watched = id;
                    navigate("preview");
                });
                action("sceneDeckOpen", [this, id] { showDeck(id); });
            } else
                action("startLiveIconLabel", [this, record] { replay(record); });
            action("restartLiveIconLabel", [this, record] { replay(record, true); });
        }
        auto remove = action("delete", [this, kind, id] {
            removeRecord(kind == "liveRows" ? "history" : kind, id);
        });
        if (kind == "liveRows" && record.get("running").flag())
            remove->setEnabled(false);
        const int next = layout->count();
        layout->addWidget(trigger, next / 2, next % 2);
        table->setCellWidget(row, table->columnCount() - 1, controls);
        table->setRowHeight(row, std::max(54, controls->sizeHint().height() + 8));
        if (oldId.toLongLong() == id)
            table->selectRow(row);
    }
    // Le azioni della pagina live restano fuori dal menu e la colonna si adatta a tutte; le altre
    // pagine lasciano che sia il contenuto a decidere.
    if (kind == "liveRows" && page == "live") {
        auto header = table->horizontalHeader();
        header->setSectionResizeMode(0, QHeaderView::Interactive);
        header->setSectionResizeMode(1, QHeaderView::Stretch);
        header->setSectionResizeMode(2, QHeaderView::Interactive);
        header->setSectionResizeMode(3, QHeaderView::Interactive);
        header->setSectionResizeMode(4, QHeaderView::Fixed);
        table->setColumnWidth(0, 130);
        table->setColumnWidth(2, 150);
        table->setColumnWidth(3, 175);
        table->setColumnWidth(4, std::max(160, liveActionsWidth + 8));
    } else
        table->horizontalHeader()->setSectionResizeMode(table->columnCount() - 1,
                                                        QHeaderView::ResizeToContents);
    if (cleanupLabel) {
        auto progress = state.get("cleanup");
        cleanupLabel->setText(
            t(progress.get("running").flag() ? "cleanupInProgress" : "cleanupSuccess")
                .replace("{0}", QLocale().toString(progress.get("deletedLives").num())));
        cleanupBar->setMaximum(std::max(1, int(progress.get("totalVideos").num())));
        cleanupBar->setValue(progress.get("deletedVideos").num());
    }
}

// Una riga di Gestione Live come MainLive: stato in pillola, titolo con la provenienza, canale,
// data di avvio e tutte le azioni visibili come icone, nell'ordine della pagina Windows.
void NativeWindow::refreshLiveRow(int row, const Json& record) {
    const long id = record.get("id").num();
    const bool running = record.get("running").flag();
    const bool isScene = record.get("isScene").flag();
    const bool isPlaylist = record.get("isPlaylist").flag();
    const bool hasDetails =
        record.get("total").num() > 1 || isScene || !record.get("error").str().empty();
    const bool hasSetting = record.get("hasSetting").flag();
    const std::string status = record.get("status").str();

    const bool live = status == "LIVE" || status == "STARTING" || status == "RECONNECTING";
    const char* statusKey = status == "STARTING"       ? "liveStatusConnecting"
                            : status == "RECONNECTING" ? "liveStatusReconnecting"
                            : live                     ? "liveStatusLive"
                            : status == "ENDED"        ? "liveStatusEnded"
                            : status == "STOPPED"      ? "liveStatusStopped"
                            : status == "ERROR"        ? "liveStatusError"
                                                       : "liveStatusOffline";
    const char* statusToken = status == "STARTING" || status == "RECONNECTING" ? "connecting"
                              : live                                           ? "live"
                              : status == "ENDED"                              ? "ended"
                              : status == "STOPPED"                            ? "stopped"
                              : status == "ERROR"                              ? "error"
                                                                               : "offline";

    // Colonna 0: lo stato, come la pillola della pagina Windows.
    auto statusCell = new QWidget(table);
    auto statusRow = new QHBoxLayout(statusCell);
    statusRow->setContentsMargins(8, 0, 8, 0);
    auto badge = new QLabel(t(statusKey), statusCell);
    badge->setToolTip(s(record.get("error")));
    statusCell->setToolTip(s(record.get("error")));
    badge->setProperty("badge", statusToken);
    badge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    statusRow->addWidget(badge);
    statusRow->addStretch();
    auto statusItem = new QTableWidgetItem;
    statusItem->setData(Qt::UserRole, qlonglong(id));
    table->setItem(row, 0, statusItem);
    table->setCellWidget(row, 0, statusCell);

    // Colonna 1: il titolo, il gettone della playlist o della scena e la provenienza.
    const QString title = s(record.get("title"));
    const QString platform = s(record.get("platform"));
    const QString source = s(record.get("source"));
    auto titleCell = new QWidget(table);
    auto titleColumn = new QVBoxLayout(titleCell);
    titleColumn->setContentsMargins(8, 4, 8, 4);
    titleColumn->setSpacing(1);
    auto titleTop = new QHBoxLayout;
    titleTop->setContentsMargins(0, 0, 0, 0);
    titleTop->setSpacing(6);
    if (isPlaylist || isScene) {
        auto token = new QLabel(titleCell);
        token->setProperty("badge", "playlist");
        token->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        if (isPlaylist)
            token->setText(t("playlistPosition")
                               .replace("{0}", QLocale().toString(record.get("position").num() + 1))
                               .replace("{1}", QLocale().toString(record.get("total").num())));
        else
            token->setText(
                t("sceneSources").replace("{0}", QLocale().toString(record.get("total").num())));
        titleTop->addWidget(token);
    }
    auto strong = new QLabel(title, titleCell);
    strong->setObjectName("rowTitle");
    strong->setToolTip(title);
    strong->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    strong->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleTop->addWidget(strong, 1);
    titleColumn->addLayout(titleTop);
    auto caption = new QLabel(
        (platform.isEmpty() ? QString{} : platform + QString::fromUtf8(" · ")) + source, titleCell);
    caption->setObjectName("rowCaption");
    caption->setToolTip(source);
    caption->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    caption->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleColumn->addWidget(caption);
    auto titleItem = new QTableWidgetItem;
    titleItem->setData(Qt::UserRole, qlonglong(id));
    table->setItem(row, 1, titleItem);
    table->setCellWidget(row, 1, titleCell);

    // Colonna 2: il canale.
    auto channelItem = new QTableWidgetItem(
        s(record.get("channelName")).isEmpty() ? QString("N/A") : s(record.get("channelName")));
    channelItem->setData(Qt::UserRole, qlonglong(id));
    table->setItem(row, 2, channelItem);

    // Colonna 3: la data di avvio, nello stesso formato della pagina Windows.
    const auto startedAt = QDateTime::fromString(s(record.get("startedAt")), Qt::ISODate);
    auto dateItem = new QTableWidgetItem(
        startedAt.isValid() ? startedAt.toLocalTime().toString("dd/MM/yyyy HH:mm:ss")
                            : QString("N/A"));
    dateItem->setData(Qt::UserRole, qlonglong(id));
    table->setItem(row, 3, dateItem);

    // Colonna 4: le azioni, tutte visibili, nell'ordine di _LiveRows.cshtml.
    auto actions = new QWidget(table);
    auto actionsRow = new QHBoxLayout(actions);
    actionsRow->setContentsMargins(2, 2, 2, 2);
    actionsRow->setSpacing(2);
    actionsRow->addStretch();
    auto iconButton = [this, actions](const QString& icon, const char* key, const QColor& color,
                                      bool enabled) {
        auto button = new QToolButton(actions);
        button->setProperty("rowAction", true);
        button->setIcon(studioIcon(icon, color));
        button->setIconSize(QSize(18, 18));
        button->setAutoRaise(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(t(key));
        button->setAccessibleName(t(key));
        button->setEnabled(enabled);
        // Un comando non disponibile resta in vista ma spento, come la pagina Windows.
        if (!enabled) {
            auto fade = new QGraphicsOpacityEffect(button);
            fade->setOpacity(0.45);
            button->setGraphicsEffect(fade);
        }
        return button;
    };
    // Riprendi diretta: solo quando in onda c'è un fermo immagine che resta.
    if (record.get("takeoverHolds").flag()) {
        auto button = new QToolButton(actions);
        button->setProperty("rowAction", true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIcon(studioIcon("play"));
        button->setIconSize(QSize(16, 16));
        button->setAutoRaise(true);
        button->setText(t("sceneResume"));
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(t("sceneOnAirNow").replace("{0}", s(record.get("takeoverLabel"))));
        button->setAccessibleName(t("sceneResume"));
        connect(button, &QToolButton::clicked, this,
                [this, id] { run([this, id] { liveCommand(id, "resume"); }); });
        actionsRow->addWidget(button);
    }
    // Il marchio della piattaforma su cui la riga è in onda, che apre la sua pagina pubblica.
    const QString known = s(record.get("platformKnown"));
    if (!known.isEmpty()) {
        auto button = new QToolButton(actions);
        button->setProperty("mark", true);
        button->setIcon(platformMark(known));
        button->setIconSize(QSize(18, 18));
        button->setAutoRaise(true);
        button->setAccessibleName(platformLabel(known));
        button->setToolTip(platformLabel(known));
        const auto url = liveUrlFor(known, s(record.get("channelName")));
        if (running && !url.isEmpty()) {
            button->setCursor(Qt::PointingHandCursor);
            connect(button, &QToolButton::clicked, this,
                    [url] { QDesktopServices::openUrl(QUrl(url)); });
        }
        actionsRow->addWidget(button);
    }
    // Dettagli: le sorgenti della scena o i video della playlist.
    if (hasDetails) {
        auto button =
            iconButton("playlist", isScene ? "sceneDetails" : "playlistDetails", {}, true);
        connect(button, &QToolButton::clicked, this, [this, record] { showLiveDetails(record); });
        actionsRow->addWidget(button);
    }
    // Collega una configurazione video: l'ingranaggio quando ce n'è già una, il collegamento quando
    // manca.
    if (hasSetting) {
        auto button = iconButton("settings", "connectVideoSetting", {}, true);
        connect(button, &QToolButton::clicked, this, [this, record] { linkProfile(record); });
        actionsRow->addWidget(button);
    } else {
        auto button = new QToolButton(actions);
        button->setProperty("rowAction", true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setIcon(studioIcon("link"));
        button->setAutoRaise(true);
        button->setText(t("connectVideoSetting"));
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(t("connectVideoSetting"));
        button->setAccessibleName(t("connectVideoSetting"));
        connect(button, &QToolButton::clicked, this, [this, record] { linkProfile(record); });
        actionsRow->addWidget(button);
    }
    // Il deck delle scene: gli stessi tasti per ogni diretta, premuti per questa.
    {
        auto button = iconButton("deck", "sceneDeckOpen", {}, true);
        connect(button, &QToolButton::clicked, this, [this, id] { showDeck(id); });
        actionsRow->addWidget(button);
    }
    // Anteprima, spot e comandi di regia solo per una diretta con un profilo collegato.
    if (hasSetting && !isPlaylist) {
        if (running) {
            auto button = iconButton("preview", "preview", {}, true);
            connect(button, &QToolButton::clicked, this, [this, id] {
                watched = id;
                navigate("preview");
            });
            actionsRow->addWidget(button);
        }
        auto spot = iconButton("spot", "playSpot", {}, running);
        connect(spot, &QToolButton::clicked, this,
                [this, id] { run([this, id] { playSpot(id); }); });
        actionsRow->addWidget(spot);
        auto stop = iconButton("stop", "stopLiveIconLabel", QColor("#ff453a"), running);
        connect(stop, &QToolButton::clicked, this, [this, id] {
            if (confirm("stopLive"))
                runAsync([this, id] {
                    studio.session(id)->stop();
                    return Json{};
                });
        });
        actionsRow->addWidget(stop);
        auto start = iconButton("play", "startLiveIconLabel", QColor("#30d158"), !running);
        connect(start, &QToolButton::clicked, this, [this, record] { replay(record); });
        actionsRow->addWidget(start);
        auto restart = iconButton("restart", "restartLiveIconLabel", QColor("#ffd60a"), true);
        connect(restart, &QToolButton::clicked, this, [this, record] { replay(record, true); });
        actionsRow->addWidget(restart);
    }
    // Elimina: l'intera playlist o la scena, oppure la sola riga.
    {
        auto button = iconButton("delete", hasDetails ? "deletePlaylist" : "deleteLiveRow",
                                 QColor("#ff453a"), !running);
        connect(button, &QToolButton::clicked, this, [this, id] { removeRecord("history", id); });
        actionsRow->addWidget(button);
    }
    actions->adjustSize();
    liveActionsWidth = std::max(liveActionsWidth, actions->sizeHint().width());
    table->setCellWidget(row, 4, actions);
    table->setRowHeight(row, 56);
}

// Uno spot è un file scelto al momento e accodato alla diretta, come il pulsante dell'anteprima.
void NativeWindow::playSpot(long id) {
    const auto file = QFileDialog::getOpenFileName(this, t("spotFile"));
    if (file.isEmpty())
        return;
    Json parameters;
    Json scene;
    scene.set("name", t("playSpot").toStdString());
    auto items = Json::array();
    Json item;
    item.set("sourceKind", "FILE");
    item.set("sourceTarget", file.toStdString());
    item.set("audioEnabled", true);
    items.push(item);
    scene.set("items", items);
    parameters.set("scene", scene);
    liveCommand(id, "spot", parameters);
}

// I controlli di regia operano sulla sessione scelta, mentre il frame viene letto direttamente dal
// disco.
// Le metriche mantengono gli stessi contatori Linux già verificati e vengono mostrate in widget
// nativi.
void NativeWindow::buildMetrics() {
    auto container = body->findChild<QWidget*>("metricsContainer");
    QGridLayout* grid = nullptr;
    if (!container) {
        container = new QWidget(body);
        container->setObjectName("metricsContainer");
        grid = new QGridLayout(container);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(12);
        bodyLayout->addWidget(container);
    } else {
        grid = qobject_cast<QGridLayout*>(container->layout());
    }

    auto values = state.get("metrics"), process = state.get("process");

    struct MetricItem {
        QString id, title, value;
        int percentage;
    };

    std::vector<MetricItem> items;

    items.push_back({"cpu", t("cpu"), formatMetric(values.get("cpu").num(-1), "%"),
                     int(values.get("cpu").num(-1))});
    items.push_back({"ram", t("ramUsed"), formatMetric(values.get("ram").num(-1), "%"),
                     int(values.get("ram").num(-1))});
    items.push_back({"swap", "SWAP", formatMetric(values.get("swap").num(-1), "%"),
                     int(values.get("swap").num(-1))});
    items.push_back({"network", "Network",
                     "↓ " + formatMetric(values.get("networkRx").num(-1) / 1024.0, " KiB/s") +
                         "\n↑ " + formatMetric(values.get("networkTx").num(-1) / 1024.0, " KiB/s"),
                     -1});
    items.push_back({"disk", t("diskFree"),
                     formatMetric(values.get("diskAvailable").num(-1) / 1073741824.0, " GiB"), -1});
    items.push_back({"uptime", t("uptime"), formatMetric(values.get("uptime").num(-1), " s"), -1});
    items.push_back({"appUsage", t("appUsage"),
                     formatMetric(process.get("cpu").num(-1), "%") + "\n" +
                         formatMetric(process.get("rssBytes").num(-1) / 1048576.0, " MiB"),
                     int(process.get("cpu").num(-1))});

    auto temperatures = values.get("temperatures");
    for (size_t i = 0; i < temperatures.size(); ++i)
        items.push_back({"temp_" + QString::number(i), s(temperatures.at(i).get("label")),
                         formatMetric(temperatures.at(i).get("celsius").num(-1), " °C"), -1});

    auto live = state.get("live");
    for (size_t i = 0; i < live.size(); ++i)
        for (auto key : {"producerMetrics", "publisherMetrics"}) {
            auto metrics = live.at(i).get(key);
            if (metrics.has("pid"))
                items.push_back(
                    {"ffmpeg_" + QString::number(metrics.get("pid").num()),
                     "FFmpeg · PID " + QString::number(metrics.get("pid").num()),
                     formatMetric(metrics.get("cpu").num(-1), "%") + "\n" +
                         formatMetric(metrics.get("rssBytes").num(-1) / 1048576.0, " MiB"),
                     int(metrics.get("cpu").num(-1))});
        }

    QSet<QString> activeIds;
    int index = 0;
    for (const auto& item : items) {
        activeIds.insert(item.id);
        auto card = container->findChild<QFrame*>(item.id);
        if (!card) {
            card = new QFrame(container);
            card->setObjectName(item.id);
            card->setProperty("class", "card");

            auto layout = new QVBoxLayout(card);
            layout->setContentsMargins(18, 18, 18, 18);
            layout->setSpacing(12);

            auto heading = new QLabel(item.title, card);
            heading->setObjectName("muted");
            heading->setWordWrap(true);
            layout->addWidget(heading);

            auto valueLabel = new QLabel(item.value, card);
            valueLabel->setObjectName("metricValue");
            layout->addWidget(valueLabel);

            auto progress = new GaugeWidget(card);
            progress->setObjectName("metricProgress");
            layout->addWidget(progress);

            layout->addStretch();

            grid->addWidget(card, index / 3, index % 3);
        } else {
            // Se esiste, aggiorniamo i dati senza ricreare il widget
            auto labels = card->findChildren<QLabel*>();
            for (auto label : labels) {
                if (label->objectName() == "muted")
                    label->setText(item.title);
                else if (label->objectName() == "metricValue")
                    label->setText(item.value);
            }
            // Riposizioniamo nella griglia per evitare buchi se qualche widget viene rimosso
            grid->addWidget(card, index / 3, index % 3);
        }

        auto progress = card->findChild<GaugeWidget*>("metricProgress");
        if (progress) {
            if (item.percentage >= 0) {
                progress->setVisible(true);
                progress->setPercentage(std::min(100, item.percentage));
            } else {
                progress->setVisible(false);
            }
        }
        card->setVisible(true);
        index++;
    }

    // Nascondi le card che non sono più attive per questa passata
    auto cards =
        container->findChildren<QFrame*>(QRegularExpression(".*"), Qt::FindDirectChildrenOnly);
    for (auto card : cards) {
        if (!activeIds.contains(card->objectName())) {
            card->setVisible(false);
        }
    }
}

// Le impostazioni aprono sezioni e dialoghi interni, conservando un’unica finestra applicativa.
void NativeWindow::buildSettings() {
    addDestinationCards({{"videoAndAudio", "profiles", "additionalVideoOptionsDescription"},
                         {"channelSettings", "channels", "channelSettingsDescription"}});
}

// Il secondo processo attende un riscontro senza aprire il database o inizializzare la GUI.
bool activateNativeApp(char** argv, const fs::path& root) {
    int argc = 1;
    QCoreApplication app(argc, argv);
    QElapsedTimer deadline;
    deadline.start();
    while (deadline.elapsed() < 4000) {
        QLocalSocket socket;
        socket.connectToServer(activationSocket(root));
        if (socket.waitForConnected(200)) {
            QByteArray reply;
            while (deadline.elapsed() < 4000) {
                if (socket.bytesAvailable() || socket.waitForReadyRead(200))
                    reply += socket.readAll();
                if (reply == "activated\n")
                    return true;
                if (socket.state() == QLocalSocket::UnconnectedState)
                    break;
            }
        }
        QThread::msleep(50);
    }
    return false;
}

// L'avvio standard crea QApplication; il socket locale permette di richiamare la stessa finestra.
int runNativeApp(int /*argc*/, char** argv, Studio& studio, const fs::path& resources) {
    // Le opzioni del motore sono già state lette; Qt riceve solo il nome eseguibile.
    int qtArgc = 1;
    QApplication app(qtArgc, argv);
    app.setApplicationName("OrbisStream");
    app.setDesktopFileName("orbis-stream");
    app.setWindowIcon(QIcon(QString::fromStdString((resources / "app-icon.png").string())));
    app.setOrganizationName("OrbisStream");
    NativeWindow window(studio, resources);
    QLocalServer activation;
    activation.setSocketOptions(QLocalServer::UserAccessOption);
    const auto socketName = activationSocket(studio.store.root);
    // Solo il proprietario del flock arriva qui: un socket residuo non è un'istanza attiva.
    QLocalServer::removeServer(socketName);
    if (!activation.listen(socketName))
        throw std::runtime_error("Impossibile attivare il richiamo della finestra: " +
                                 activation.errorString().toStdString());
    QObject::connect(&activation, &QLocalServer::newConnection, &window, [&] {
        while (auto socket = activation.nextPendingConnection()) {
            if (window.isMinimized())
                window.setWindowState(window.windowState() & ~Qt::WindowMinimized);
            window.show();
            window.raise();
            window.activateWindow();
            if (auto modal = QApplication::activeModalWidget()) {
                modal->raise();
                modal->activateWindow();
            }
            QApplication::alert(&window);
            QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            socket->write("activated\n");
            socket->disconnectFromServer();
        }
    });
    QTimer shutdownTimer;
    QObject::connect(&shutdownTimer, &QTimer::timeout, &app, [&] {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGTERM);
        sigaddset(&set, SIGINT);
        timespec immediate{0, 0};
        if (sigtimedwait(&set, nullptr, &immediate) > 0)
            app.quit();
    });
    shutdownTimer.start(100);
    window.show();
    const int result = app.exec();
    activation.close();
    // NativeWindow attende i lavori pendenti e arresta tutte le sessioni nel distruttore.
    return result;
}
} // namespace orbis

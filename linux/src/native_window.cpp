#include "native_window.hpp"
#include "history_cleanup_service.hpp"
#include "live_rows.hpp"
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
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>
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
} // namespace

// La finestra richiama il motore direttamente: nessun server HTTP, browser o WebView è avviato.
NativeWindow::NativeWindow(Studio& studio, fs::path resources)
    : studio(studio), resources(std::move(resources)),
      translations(QString::fromStdString((this->resources / "ui").string())),
      proxies(studio.store.root / "proxies") {
    applyNativeTheme();
    setObjectName("nativeWindow");
    setWindowTitle(QString("OrbisStream · %1").arg(ORBIS_VERSION));
    resize(1380, 900);
    setMinimumSize(960, 640);
    setWindowIcon(studioIcon("live"));
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
    horizontal->setContentsMargins(0, 0, 0, 0);
    horizontal->setSpacing(0);
    auto sidebar = new QWidget(central);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(252);
    auto side = new QVBoxLayout(sidebar);
    side->setContentsMargins(16, 24, 16, 18);
    side->setSpacing(12);
    auto brandRow = new QHBoxLayout;
    auto mark = new QLabel("O", sidebar);
    mark->setObjectName("brandMark");
    mark->setAlignment(Qt::AlignCenter);
    mark->setFixedSize(42, 42);
    brandRow->addWidget(mark);
    auto brand = new QLabel("OrbisStream", sidebar);
    brand->setObjectName("brand");
    brandRow->addWidget(brand, 1);
    side->addLayout(brandRow);
    side->addSpacing(12);
    navigation = new QListWidget(sidebar);
    navigation->setObjectName("navigation");
    navigation->setIconSize(QSize(22, 22));
    navigation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigationKeys = {"dashboard",   "liveManagement", "preview", "layouts",
                      "taskManager", "settings",       "manual"};
    navigationPages = {"dashboard", "live", "preview", "layouts", "activity", "settings", "manual"};
    side->addWidget(navigation, 1);
    languagePicker = new QComboBox(sidebar);
    languagePicker->setObjectName("languagePicker");
    for (const auto& language : translations.languages())
        languagePicker->addItem(NativeTranslations::languageName(language), language);
    languagePicker->setCurrentIndex(languagePicker->findData(translations.language()));
    languagePicker->setAccessibleName(t("settings"));
    side->addWidget(languagePicker);
    auto version = new QLabel(QString("OrbisStream  /  %1").arg(ORBIS_VERSION), sidebar);
    version->setObjectName("muted");
    side->addWidget(version);
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
    connect(navigation, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < navigationPages.size())
            navigate(navigationPages.at(row));
    });
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
void NativeWindow::rebuild() {
    table = nullptr;
    search = nullptr;
    channelFilter = statusFilter = platformFilter = livePicker = nullptr;
    preview = previewFacts = cleanupLabel = pageLabel = emptyState = nullptr;
    liveCount = channelCount = cpuValue = memoryValue = dashboardStatus = nullptr;
    cleanupBar = nullptr;
    while (auto item = bodyLayout->takeAt(0)) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
    navigation->blockSignals(true);
    navigation->clear();
    for (int i = 0; i < navigationKeys.size(); ++i) {
        const auto label = t(navigationKeys.at(i).toUtf8().constData());
        auto item = new QListWidgetItem(studioIcon(navigationPages.at(i)), label, navigation);
        item->setToolTip(label);
    }
    int selected = navigationPages.indexOf(page);
    if (selected < 0)
        selected = navigationPages.indexOf(
            page == "scenes"                                               ? "layouts"
            : page == "playlist" || page == "buttons" || page == "history" ? "live"
                                                                           : "settings");
    navigation->setCurrentRow(selected);
    navigation->blockSignals(false);
    const QMap<QString, const char*> titles = {
        {"dashboard", "dashboard"},    {"live", "liveManagement"},
        {"history", "liveHistory"},    {"preview", "preview"},
        {"layouts", "layouts"},        {"scenes", "sceneStrip"},
        {"activity", "taskManager"},   {"manual", "manual"},
        {"settings", "settings"},      {"channels", "channelSettings"},
        {"profiles", "videoSettings"}, {"playlist", "nativePlaylistLibrary"},
        {"overlays", "overlays"},      {"buttons", "sceneDeck"},
        {"scene-media", "sceneMedia"}};
    auto title = new QLabel(t(titles.value(page, "dashboard")), body);
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
        {"history", "liveHistoryDescription"},
        {"preview", "previewDescription"},
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
    if (page == "live" || page == "preview") {
        auto actions = new QWidget(heading);
        auto row = new QHBoxLayout(actions);
        row->setContentsMargins(12, 0, 0, 0);
        // Come la pagina Windows: sblocca (solo con video bloccati), avvia playlist, avvia live.
        if (page == "live") {
            const auto videos = studio.store.list("videos");
            for (size_t i = 0; i < videos.size(); ++i)
                if (videos.at(i).get("isLocked").flag()) {
                    addAction(
                        "unLockVideo",
                        [this] {
                            const auto rows = studio.store.list("videos");
                            for (size_t j = 0; j < rows.size(); ++j) {
                                auto video = rows.at(j).clone();
                                video.set("isLocked", false);
                                studio.store.save("videos", video, video.get("id").num());
                            }
                            sample();
                        },
                        actions, "unlock");
                    break;
                }
        }
        addAction("startPlaylistButton", [this] { startWizard(true); }, actions, "playlist");
        addAction("startLiveButton", [this] { startWizard(); }, actions, "play");
        headingLayout->addWidget(actions);
    }
    bodyLayout->addWidget(heading);
    if (page == "dashboard")
        buildDashboard();
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
    layout->addWidget(previous);
    layout->addWidget(pageLabel);
    layout->addWidget(next);
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
    auto destinations = new QWidget(body);
    auto grid = new QGridLayout(destinations);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(12);
    grid->setSizeConstraint(QLayout::SetMinimumSize);
    destinations->setMinimumHeight(((entries.size() + 1) / 2) * 182 - 12);
    int index = 0;
    for (const auto& entry : entries) {
        auto button = new QPushButton(destinations);
        button->setObjectName(entry[0]);
        button->setProperty("card", true);
        button->setCursor(Qt::PointingHandCursor);
        button->setAccessibleName(t(entry[0].toUtf8().constData()));
        auto content = new QVBoxLayout(button);
        content->setContentsMargins(20, 18, 20, 18);
        auto name = new QLabel(t(entry[0].toUtf8().constData()), button);
        name->setObjectName("cardTitle");
        name->setWordWrap(true);
        auto description = new QLabel(t(entry[2].toUtf8().constData()), button);
        description->setObjectName("muted");
        description->setWordWrap(true);
        auto icon = new QLabel(button);
        icon->setPixmap(studioIcon(entry[1]).pixmap(24, 24));
        for (auto label : {icon, name, description}) {
            label->setAttribute(Qt::WA_TransparentForMouseEvents);
            content->addWidget(label);
        }
        content->addStretch();
        button->setMinimumHeight(170);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect(button, &QPushButton::clicked, this,
                [this, target = entry[1]] { navigate(target); });
        grid->addWidget(button, index / 2, index % 2);
        ++index;
    }
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    bodyLayout->addWidget(destinations);
}

// La dashboard mette prima gli accessi descritti dal manuale, poi i contatori della macchina.
void NativeWindow::buildDashboard() {
    auto section = [this](const char* key) {
        auto label = new QLabel(t(key), body);
        label->setObjectName("sectionLabel");
        bodyLayout->addWidget(label);
    };
    section("dashboardSectionLive");
    addDestinationCards({{"liveManagement", "live", "liveManagementDescription"},
                         {"preview", "preview", "previewDescription"},
                         {"layouts", "layouts", "layoutsDescription"},
                         {"liveHistory", "history", "liveHistoryDescription"}});
    section("dashboardSectionSettings");
    addDestinationCards({{"videoAndAudio", "profiles", "additionalVideoOptionsDescription"},
                         {"channelSettings", "channels", "channelSettingsDescription"}});
    section("dashboardSectionSystem");
    addDestinationCards({{"taskManager", "activity", "taskManagerDescription"},
                         {"manual", "manual", "manualDescription"}});

    auto metrics = new QWidget(body);
    auto metricGrid = new QGridLayout(metrics);
    metricGrid->setContentsMargins(0, 0, 0, 0);
    metricGrid->setSpacing(12);
    auto metric = [&](int column, const char* key, const QString& icon) {
        auto card = new QFrame(metrics);
        card->setObjectName("card");
        auto layout = new QVBoxLayout(card);
        layout->setContentsMargins(18, 18, 18, 18);
        layout->setSpacing(12);
        auto heading = new QHBoxLayout;
        auto label = new QLabel(t(key), card);
        label->setWordWrap(true);
        label->setObjectName("muted");
        heading->addWidget(label, 1);
        auto symbol = new QLabel(card);
        symbol->setPixmap(studioIcon(icon).pixmap(22, 22));
        heading->addWidget(symbol);
        layout->addLayout(heading);
        auto value = new QLabel("—", card);
        value->setObjectName("metricValue");
        layout->addWidget(value);
        metricGrid->addWidget(card, 0, column);
        metricGrid->setColumnStretch(column, 1);
        return value;
    };
    liveCount = metric(0, "liveStatusLive", "live");
    channelCount = metric(1, "channel", "channels");
    cpuValue = metric(2, "cpu", "activity");
    memoryValue = metric(3, "ramUsed", "profiles");
    bodyLayout->addWidget(metrics);

    auto statusRow = new QWidget(body);
    auto statusLayout = new QHBoxLayout(statusRow);
    statusLayout->setContentsMargins(0, 2, 0, 2);
    auto statusTitle = new QLabel(t("liveStatus"), statusRow);
    statusTitle->setObjectName("sectionLabel");
    statusLayout->addWidget(statusTitle);
    statusLayout->addStretch();
    dashboardStatus = new QLabel(t("loading"), statusRow);
    dashboardStatus->setObjectName("statusPill");
    statusLayout->addWidget(dashboardStatus);
    bodyLayout->addWidget(statusRow);

    updateDashboard();
}

// I valori assenti rimangono trattini fino al primo campione; le dirette concluse non sono contate.
void NativeWindow::updateDashboard() {
    if (!liveCount || !state.has("live"))
        return;
    int running = 0;
    const auto live = state.get("live");
    for (size_t i = 0; i < live.size(); ++i) {
        const auto status = live.at(i).get("status").str();
        if (status == "LIVE" || status == "STARTING" || status == "RECONNECTING")
            ++running;
    }
    liveCount->setText(QLocale().toString(running));
    channelCount->setText(QLocale().toString(qlonglong(state.get("channels").size())));
    cpuValue->setText(formatMetric(state.get("metrics").get("cpu").num(-1), "%"));
    memoryValue->setText(formatMetric(state.get("metrics").get("ram").num(-1), "%"));
    dashboardStatus->setText(running ? t("liveStatusLive") + " · " + QLocale().toString(running)
                                     : t("noLiveRunning"));
}

// Filtri e ricerca restano fuori dalle righe per conservare il focus durante il campionamento.
void NativeWindow::buildChannels() {
    addAction("addConfiguration", [this] { editChannel(); });
    search = new QLineEdit(body);
    search->setObjectName("search");
    search->setPlaceholderText(t("search"));
    bodyLayout->addWidget(search);
    connect(search, &QLineEdit::textChanged, this, [this] { refreshRows(); });
    platformFilter = new QComboBox(body);
    platformFilter->addItem("—", "");
    for (const auto& platform : {"Custom", "Twitch", "YouTube", "Kick", "Facebook", "TikTok"})
        platformFilter->addItem(platform, platform);
    bodyLayout->addWidget(platformFilter);
    connect(platformFilter, &QComboBox::currentIndexChanged, this, [this] { refreshRows(); });
    table = makeTable({t("channel"), t("platform"), t("streamUrl"), t("active"), t("settings")});
}

// Mostra i profili personalizzati; i profili seed restano disponibili come preset nei dialoghi.
void NativeWindow::buildProfiles() {
    addAction("addNewAudioVideoConfiguration", [this] { editProfile(); });
    search = new QLineEdit(body);
    search->setPlaceholderText(t("search"));
    bodyLayout->addWidget(search);
    connect(search, &QLineEdit::textChanged, this, [this] { refreshRows(); });
    table = makeTable(
        {t("title"), t("resolution"), t("frameRate"), t("bitrate"), t("active"), t("settings")});
}

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
        connect(back, &QToolButton::clicked, this, [this] { navigate("live"); });
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
                studioIcon(remove ? "delete" : "edit", QColor(remove ? "#ff453a" : "#f5f5f7")));
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
void NativeWindow::buildLive(bool history) {
    if (!history) {
        addAction("liveHistory", [this] { navigate("history"); });
        addAction("sceneDeck", [this] { navigate("buttons"); });
        addAction("nativePlaylistLibrary", [this] { navigate("playlist"); });
    } else {
        addAction("liveManagement", [this] { navigate("live"); });
        addAction("nativeExport", [this] { exportData(); });
        addAction("autoCleanup", [this] {
            // Le regole restano sul canale attivo, esattamente come nella pagina originale.
            auto channels = studio.store.list("channels");
            for (size_t i = 0; i < channels.size(); ++i)
                if (channels.at(i).get("isActive").flag(true)) {
                    editChannel(channels.at(i));
                    return;
                }
        });
        addAction("runCleanupNow", [this] {
            bool ok = false;
            const QStringList labels = {t("olderThanYesterday"), t("olderThan1Month"),
                                        t("olderThan3Months"),   t("olderThan6Months"),
                                        t("olderThan1Year"),     t("olderThan2Years"),
                                        t("olderThanNow")};
            auto dialog = new QDialog(this);
            dialog->setWindowTitle(t("autoCleanup"));
            auto layout = new QVBoxLayout(dialog);
            auto choice = new QComboBox(dialog);
            choice->addItems(labels);
            layout->addWidget(choice);
            auto buttons =
                new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
            layout->addWidget(buttons);
            connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
            ok = dialog->exec() == QDialog::Accepted;
            const int index = choice->currentIndex();
            delete dialog;
            if (ok && confirm("deleteLiveHistoryConfirm"))
                studio.historyCleanup->tryStart(QList<int>{0, 1, 3, 6, 12, 24, -1}.at(index));
        });
        cleanupLabel = new QLabel(body);
        cleanupBar = new QProgressBar(body);
        bodyLayout->addWidget(cleanupLabel);
        bodyLayout->addWidget(cleanupBar);
    }
    auto filters = new QWidget(body);
    auto row = new QHBoxLayout(filters);
    statusFilter = new QComboBox(filters);
    statusFilter->addItem("—", "");
    for (const auto& entry : QList<QPair<const char*, QString>>{{"liveStatusLive", "LIVE"},
                                                                {"liveStatusOffline", "OFFLINE"},
                                                                {"liveStatusEnded", "ENDED"},
                                                                {"liveStatusError", "ERROR"},
                                                                {"liveStatusStopped", "STOPPED"}})
        statusFilter->addItem(t(entry.first), entry.second);
    channelFilter = new QComboBox(filters);
    channelFilter->setObjectName("channelFilter");
    channelFilter->addItem("—", "");
    // La pagina Windows non ha una casella di ricerca: lo storico, che ne ha una, la conserva.
    if (history) {
        search = new QLineEdit(filters);
        search->setPlaceholderText(t("searchByTitle"));
        search->setObjectName("search");
    }
    platformFilter = new QComboBox(filters);
    platformFilter->setObjectName("platformFilter");
    platformFilter->addItem("—", "");
    for (auto platform : {"Custom", "Twitch", "YouTube", "Kick", "Facebook", "TikTok"})
        platformFilter->addItem(platform, platform);
    connect(platformFilter, &QComboBox::currentIndexChanged, this, [this] {
        currentPage = 0;
        refreshRows();
    });
    row->setContentsMargins(0, 0, 0, 0);
    auto fields = QList<QPair<const char*, QWidget*>>{
        {"liveStatus", statusFilter}, {"channel", channelFilter}, {"platform", platformFilter}};
    if (history)
        fields.append({"searchByTitle", search});
    for (const auto& field : fields) {
        auto column = new QVBoxLayout;
        auto label = new QLabel(t(field.first), filters);
        label->setObjectName("sectionLabel");
        label->setBuddy(field.second);
        field.second->setAccessibleName(t(field.first));
        column->addWidget(label);
        column->addWidget(field.second);
        row->addLayout(column, 1);
    }
    bodyLayout->addWidget(filters);
    connect(statusFilter, &QComboBox::currentIndexChanged, this, [this] {
        currentPage = 0;
        refreshRows();
    });
    connect(channelFilter, &QComboBox::currentIndexChanged, this, [this] {
        currentPage = 0;
        refreshRows();
    });
    if (search)
        connect(search, &QLineEdit::textChanged, this, [this] {
            currentPage = 0;
            refreshRows();
        });
    if (history)
        table = makeTable({t("title"), t("liveStatus"), t("channel"), t("platform"), t("startDate"),
                           t("settings")});
    else {
        // Come MainLive: stato, titolo, canale, data di avvio e le azioni, senza intestazione.
        auto startDate = t("startDate");
        while (startDate.endsWith(' ') || startDate.endsWith(':'))
            startDate.chop(1);
        table = makeTable({t("liveStatus"), t("title"), t("channel"), startDate, ""});
    }
    addPager(history ? 20 : 10);
}

// Ogni riga conserva l'ID nei dati Qt: ordinare o filtrare non cambia l'elemento su cui agisce un
// pulsante.
void NativeWindow::refreshRows() {
    // Non distrugge menu o selettori mentre l’utente sta scegliendo un’azione.
    if (QApplication::activePopupWidget())
        return;
    if (page == "layouts") {
        refreshLayouts();
        return;
    }
    if (page == "dashboard") {
        updateDashboard();
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
    const bool hasDetails = record.get("total").num() > 1 || isScene;
    const bool hasSetting = record.get("hasSetting").flag();
    const std::string status = record.get("status").str();

    const bool live = status == "LIVE" || status == "STARTING" || status == "RECONNECTING";
    const char* statusKey = live                  ? "liveStatusLive"
                            : status == "ENDED"   ? "liveStatusEnded"
                            : status == "STOPPED" ? "liveStatusStopped"
                            : status == "ERROR"   ? "liveStatusError"
                                                  : "liveStatusOffline";
    const char* statusToken = live                  ? "live"
                              : status == "ENDED"   ? "ended"
                              : status == "STOPPED" ? "stopped"
                              : status == "ERROR"   ? "error"
                                                    : "offline";

    // Colonna 0: lo stato, come la pillola della pagina Windows.
    auto statusCell = new QWidget(table);
    auto statusRow = new QHBoxLayout(statusCell);
    statusRow->setContentsMargins(8, 0, 8, 0);
    auto badge = new QLabel(t(statusKey), statusCell);
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
void NativeWindow::buildPreview() {
    livePicker = new QComboBox(body);
    livePicker->setObjectName("livePicker");
    auto pickerLabel = new QLabel(t("selectLiveToWatch"), body);
    pickerLabel->setBuddy(livePicker);
    bodyLayout->addWidget(pickerLabel);
    bodyLayout->addWidget(livePicker);
    preview = new QLabel(t("previewWaiting"), body);
    preview->setObjectName("livePreview");
    preview->setMinimumSize(480, 270);
    preview->setAlignment(Qt::AlignCenter);
    bodyLayout->addWidget(preview);
    previewFacts = new QLabel(body);
    bodyLayout->addWidget(previewFacts);
    connect(livePicker, &QComboBox::currentIndexChanged, this, [this] {
        watched = livePicker->currentData().toLongLong();
        updatePreview();
    });
    addAction("sourceVolumes", [this] { showMixer(watched); });
    addAction("sceneDeck", [this] { showDeck(watched); });
    addAction("videoParameters", [this] {
        Json h;
        h.set("id", double(watched));
        h.set("running", true);
        h.set("request", Json{});
        linkProfile(h);
    });
    addAction("playSpot", [this] { playSpot(watched); });
    addAction("nativeNextVideo", [this] { liveCommand(watched, "next"); });
    addAction("sceneResume", [this] { liveCommand(watched, "resume"); });
    addAction("liveOnPlatform", [this] {
        auto session = (studio.session(watched) ? studio.session(watched)->snapshot() : Json{});
        auto channel = studio.store.get("channels", session.get("channelId").num());
        auto player = platformPlayer(channel);
        if (player.get("publicUrl").str().empty())
            throw std::runtime_error("No public URL");
        QDesktopServices::openUrl(QUrl(s(player.get("publicUrl"))));
    });
    addAction("stopLiveIconLabel", [this] {
        if (confirm("stopLive"))
            runAsync([this, id = watched] {
                if (auto session = studio.session(id))
                    session->stop();
                return Json{};
            });
    });
}

// Legge il frame JPEG generato dal motore come immagine Qt; nessun player HTML o indirizzo HTTP.
void NativeWindow::updatePreview() {
    if (QApplication::activePopupWidget())
        return;
    if (!preview || !livePicker)
        return;
    auto live = state.get("live");
    livePicker->blockSignals(true);
    livePicker->clear();
    for (size_t i = 0; i < live.size(); ++i) {
        auto item = live.at(i);
        const auto status = item.get("status").str();
        if (status == "LIVE" || status == "STARTING" || status == "RECONNECTING")
            livePicker->addItem(s(item.get("channelName")) + " · " +
                                    QString::number(item.get("id").num()),
                                qlonglong(item.get("id").num()));
    }
    auto picked = livePicker->findData(qlonglong(watched));
    livePicker->setCurrentIndex(picked >= 0 ? picked : 0);
    watched = livePicker->currentData().toLongLong();
    livePicker->blockSignals(false);
    for (auto button : body->findChildren<QPushButton*>()) {
        if (button->objectName() != "startLiveButton" &&
            button->objectName() != "startPlaylistButton")
            button->setEnabled(watched != 0);
    }
    if (!watched) {
        preview->clear();
        preview->setText(t("noLiveRunning") + "\n\n" + t("startALiveHere"));
        previewFacts->clear();
        return;
    }
    const auto path = studio.store.root / "previews" / (std::to_string(watched) + ".jpg");
    QPixmap frame(QString::fromStdString(path.string()));
    if (!frame.isNull())
        preview->setPixmap(
            frame.scaled(preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    for (size_t i = 0; i < live.size(); ++i)
        if (live.at(i).get("id").num() == watched) {
            auto profile = live.at(i).get("profile");
            previewFacts->setText(QString("%1 × %2 · %3 fps · %4 kbps")
                                      .arg(profile.get("videoWidth").num())
                                      .arg(profile.get("videoHeight").num())
                                      .arg(profile.get("frameRate").num())
                                      .arg(live.at(i).get("outputKbps").num(), 0, 'f', 0));
        }
}

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

    items.push_back({"cpu", t("cpu"), formatMetric(values.get("cpu").num(-1), "%"), int(values.get("cpu").num(-1))});
    items.push_back({"ram", t("ramUsed"), formatMetric(values.get("ram").num(-1), "%"), int(values.get("ram").num(-1))});
    items.push_back({"swap", "SWAP", formatMetric(values.get("swap").num(-1), "%"), int(values.get("swap").num(-1))});
    items.push_back({"network", "Network",
                     "↓ " + formatMetric(values.get("networkRx").num(-1) / 1024.0, " KiB/s") + "\n↑ " +
                     formatMetric(values.get("networkTx").num(-1) / 1024.0, " KiB/s"), -1});
    items.push_back({"disk", t("diskFree"), formatMetric(values.get("diskAvailable").num(-1) / 1073741824.0, " GiB"), -1});
    items.push_back({"uptime", t("uptime"), formatMetric(values.get("uptime").num(-1), " s"), -1});
    items.push_back({"appUsage", t("appUsage"), formatMetric(process.get("cpu").num(-1), "%") + "\n" +
                     formatMetric(process.get("rssBytes").num(-1) / 1048576.0, " MiB"), int(process.get("cpu").num(-1))});

    auto temperatures = values.get("temperatures");
    for (size_t i = 0; i < temperatures.size(); ++i)
        items.push_back({"temp_" + QString::number(i), s(temperatures.at(i).get("label")),
                         formatMetric(temperatures.at(i).get("celsius").num(-1), " °C"), -1});

    auto live = state.get("live");
    for (size_t i = 0; i < live.size(); ++i)
        for (auto key : {"producerMetrics", "publisherMetrics"}) {
            auto metrics = live.at(i).get(key);
            if (metrics.has("pid"))
                items.push_back({"ffmpeg_" + QString::number(metrics.get("pid").num()),
                                 "FFmpeg · PID " + QString::number(metrics.get("pid").num()),
                                 formatMetric(metrics.get("cpu").num(-1), "%") + "\n" +
                                 formatMetric(metrics.get("rssBytes").num(-1) / 1048576.0, " MiB"), int(metrics.get("cpu").num(-1))});
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

            auto progress = new QProgressBar(card);
            progress->setObjectName("metricProgress");
            progress->setRange(0, 100);
            progress->setTextVisible(false);
            layout->addWidget(progress);

            grid->addWidget(card, index / 3, index % 3);
        } else {
            // Se esiste, aggiorniamo i dati senza ricreare il widget
            auto labels = card->findChildren<QLabel*>();
            for (auto label : labels) {
                if (label->objectName() == "muted") label->setText(item.title);
                else if (label->objectName() == "metricValue") label->setText(item.value);
            }
            // Riposizioniamo nella griglia per evitare buchi se qualche widget viene rimosso
            grid->addWidget(card, index / 3, index % 3);
        }

        auto progress = card->findChild<QProgressBar*>("metricProgress");
        if (progress) {
            if (item.percentage >= 0) {
                progress->setVisible(true);
                progress->setValue(std::min(100, item.percentage));
            } else {
                progress->setVisible(false);
            }
        }
        card->setVisible(true);
        index++;
    }

    // Nascondi le card che non sono più attive per questa passata
    auto cards = container->findChildren<QFrame*>(QRegularExpression(".*"), Qt::FindDirectChildrenOnly);
    for (auto card : cards) {
        if (!activeIds.contains(card->objectName())) {
            card->setVisible(false);
        }
    }
}

// Le impostazioni aprono sezioni e dialoghi interni, conservando un’unica finestra applicativa.
void NativeWindow::buildSettings() {
    addDestinationCards({{"videoAndAudio", "profiles", "additionalVideoOptionsDescription"},
                         {"channelSettings", "channels", "channelSettingsDescription"},
                         {"sceneDeck", "buttons", "sceneDeckHint"},
                         {"overlays", "overlays", "overlaysHint"}});
    addAction("sceneMedia", [this] { navigate("scene-media"); });
    addAction("nativeExport", [this] { exportData(); });
    addAction("nativeImport", [this] { importData(); });
    addAction("about", [this] {
        QMessageBox::information(
            this, t("about"),
            QString("OrbisStream %1\nC++ · Qt 6\n%2").arg(ORBIS_VERSION, t("aboutVersion")));
    });
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

#include "native_window.hpp"
#include "history_cleanup_service.hpp"
#include "live_rows.hpp"
#include "native_theme.hpp"
#include "scene_service.hpp"
#include "version.hpp"
#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFrame>
#include <QFutureWatcher>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>
#include <QTableWidget>
#include <QThreadPool>
#include <QTimer>
#include <QToolButton>
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

QString s(const Json& value) {
    return QString::fromStdString(value.str());
}

// I contatori non disponibili restano distinti da uno zero misurato.
QString formatMetric(double value, const QString& unit) {
    return value < 0 ? QString::fromUtf8("—") : QLocale().toString(value, 'f', 1) + unit;
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
    navigationKeys = {"dashboard",   "liveManagement",  "preview",       "sceneStrip",
                      "layouts",     "channelSettings", "videoSettings", "liveHistory",
                      "taskManager", "manual",          "settings"};
    navigationPages = {"dashboard", "live",    "preview",  "scenes", "layouts", "channels",
                       "profiles",  "history", "activity", "manual", "settings"};
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
    else
        event->accept();
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
    preview = previewFacts = cleanupLabel = pageLabel = nullptr;
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
        selected =
            navigationPages.indexOf(page == "playlist" || page == "buttons" ? "live" : "settings");
    navigation->setCurrentRow(selected);
    navigation->blockSignals(false);
    const QMap<QString, const char*> titles = {
        {"dashboard", "dashboard"},    {"live", "liveManagement"},
        {"history", "liveHistory"},    {"preview", "preview"},
        {"layouts", "layouts"},        {"scenes", "sceneStrip"},
        {"activity", "taskManager"},   {"manual", "manual"},
        {"settings", "settings"},      {"channels", "channelSettings"},
        {"profiles", "videoSettings"}, {"playlist", "startPlaylistButton"},
        {"overlays", "overlays"},      {"buttons", "sceneDeck"},
        {"scene-media", "sceneMedia"}};
    auto title = new QLabel(t(titles.value(page, "dashboard")), body);
    title->setObjectName("pageTitle");
    auto heading = new QWidget(body);
    auto headingLayout = new QHBoxLayout(heading);
    headingLayout->setContentsMargins(0, 0, 0, 8);
    headingLayout->addWidget(title, 1);
    if (page != "dashboard" && page != "live") {
        auto start = new QPushButton(studioIcon("play"), t("startLiveButton"), heading);
        start->setProperty("primary", true);
        connect(start, &QPushButton::clicked, this, [this] { run([this] { startWizard(); }); });
        headingLayout->addWidget(start);
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
void NativeWindow::addAction(const char* key, const std::function<void()>& action,
                             QWidget* parent) {
    auto button = new QPushButton(t(key), parent ? parent : body);
    button->setObjectName(key);
    const QString name(key);
    button->setProperty("primary", name.startsWith("add") || name == "startLiveButton" ||
                                       name == "newLayout" || name == "composeScene");
    button->setProperty("danger", name.contains("stop", Qt::CaseInsensitive) || name == "delete");
    if (name.startsWith("add") || name == "newLayout")
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
    return result;
}

// Le azioni di pagina restano fuori dalla tabella e i filtri non si perdono durante il polling.
void NativeWindow::addPager(int size) {
    pageSize = size;
    auto row = new QWidget(body);
    auto layout = new QHBoxLayout(row);
    auto previous = new QPushButton(t("back"), row), next = new QPushButton(t("next"), row);
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

// La dashboard combina comandi operativi e contatori aggiornati dallo stesso stato del motore.
void NativeWindow::buildDashboard() {
    auto hero = new QFrame(body);
    hero->setObjectName("hero");
    auto heroLayout = new QVBoxLayout(hero);
    heroLayout->setContentsMargins(26, 24, 26, 24);
    heroLayout->setSpacing(16);
    auto eyebrow = new QLabel("ORBIS / STREAM STUDIO", hero);
    eyebrow->setObjectName("sectionLabel");
    heroLayout->addWidget(eyebrow);
    auto heroTitle = new QLabel(t("liveManagement"), hero);
    heroTitle->setObjectName("heroTitle");
    heroLayout->addWidget(heroTitle);
    auto description = new QLabel(t("dashboardDescription"), hero);
    description->setWordWrap(true);
    description->setObjectName("muted");
    heroLayout->addWidget(description);
    auto heroActions = new QWidget(hero);
    auto heroButtons = new QHBoxLayout(heroActions);
    heroButtons->setContentsMargins(0, 4, 0, 0);
    addAction("startLiveButton", [this] { startWizard(); }, heroActions);
    addAction("startPlaylistButton", [this] { startWizard(true); }, heroActions);
    heroButtons->addStretch();
    heroLayout->addWidget(heroActions);
    bodyLayout->addWidget(hero);

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

    auto destinations = new QWidget(body);
    auto grid = new QGridLayout(destinations);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(12);
    int index = 0;
    for (const auto& entry : QList<QPair<const char*, QString>>{{"liveManagement", "live"},
                                                                {"preview", "preview"},
                                                                {"layouts", "layouts"},
                                                                {"liveHistory", "history"},
                                                                {"channelSettings", "channels"},
                                                                {"videoSettings", "profiles"}}) {
        auto button = new QPushButton(studioIcon(entry.second), t(entry.first), destinations);
        button->setObjectName(entry.first);
        button->setProperty("card", true);
        button->setIconSize(QSize(26, 26));
        button->setMinimumHeight(78);
        connect(button, &QPushButton::clicked, this,
                [this, target = entry.second] { navigate(target); });
        grid->addWidget(button, index / 2, index % 2);
        ++index;
    }
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    bodyLayout->addWidget(destinations);
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
    addAction(layouts ? "newLayout" : "composeScene", [this, layouts] {
        Json scene;
        scene.set("name", t(layouts ? "newLayout" : "composeScene").toStdString());
        scene.set("width", 1280.0);
        scene.set("height", 720.0);
        scene.set("isLayout", layouts);
        scene.set("items", Json::array());
        editScene(scene);
    });
    addAction(layouts ? "sceneStrip" : "layouts",
              [this, layouts] { navigate(layouts ? "scenes" : "layouts"); });
    addAction("overlays", [this] { navigate("overlays"); });
    table = makeTable({t("title"), t("description"), t("resolution"), t("settings")});
}

// Mantiene filtri e pulizia del flusso originale, con pagine da dieci dirette o venti storici.
void NativeWindow::buildLive(bool history) {
    if (!history) {
        addAction("startLiveButton", [this] { startWizard(); });
        addAction("startPlaylistButton", [this] { startWizard(true); });
        addAction("liveHistory", [this] { navigate("history"); });
        addAction("sceneDeck", [this] { navigate("buttons"); });
        addAction("startPlaylistTitle", [this] { navigate("playlist"); });
    } else {
        addAction("liveManagement", [this] { navigate("live"); });
        addAction("save", [this] { exportData(); });
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
    search = new QLineEdit(filters);
    search->setPlaceholderText(t("searchByTitle"));
    search->setObjectName("search");
    platformFilter = new QComboBox(filters);
    platformFilter->setObjectName("platformFilter");
    platformFilter->addItem("—", "");
    for (auto platform : {"Custom", "Twitch", "YouTube", "Kick", "Facebook", "TikTok"})
        platformFilter->addItem(platform, platform);
    connect(platformFilter, &QComboBox::currentIndexChanged, this, [this] {
        currentPage = 0;
        refreshRows();
    });
    row->addWidget(statusFilter);
    row->addWidget(channelFilter);
    row->addWidget(platformFilter);
    row->addWidget(search);
    bodyLayout->addWidget(filters);
    connect(statusFilter, &QComboBox::currentIndexChanged, this, [this] {
        currentPage = 0;
        refreshRows();
    });
    connect(channelFilter, &QComboBox::currentIndexChanged, this, [this] {
        currentPage = 0;
        refreshRows();
    });
    connect(search, &QLineEdit::textChanged, this, [this] {
        currentPage = 0;
        refreshRows();
    });
    table = makeTable(
        {t("title"), t("liveStatus"), t("channel"), t("platform"), t("startDate"), t("settings")});
    addPager(history ? 20 : 10);
}

// Ogni riga conserva l'ID nei dati Qt: ordinare o filtrare non cambia l'elemento su cui agisce un
// pulsante.
void NativeWindow::refreshRows() {
    // Non distrugge menu o selettori mentre l’utente sta scegliendo un’azione.
    if (QApplication::activePopupWidget())
        return;
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
        pageLabel->setText(t("positionOf")
                               .replace("{0}", QLocale().toString(currentPage + 1))
                               .replace("{1}", QLocale().toString(total)));
        const auto first = std::min(selected.size(), size_t(currentPage * pageSize)),
                   last = std::min(selected.size(), first + pageSize);
        selected = std::vector<Json>(selected.begin() + first, selected.begin() + last);
    }
    const auto oldId = table->currentRow() >= 0
                           ? table->item(table->currentRow(), 0)->data(Qt::UserRole)
                           : QVariant{};
    table->setRowCount(0);
    for (const auto& record : selected) {
        const long id = record.get("id").num();
        const int row = table->rowCount();
        table->insertRow(row);
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
        auto layout = new QHBoxLayout(controls);
        layout->setContentsMargins(2, 2, 2, 2);
        // Un menu Qt mantiene tutte le azioni accessibili anche nelle colonne strette o in lingue
        // con testi lunghi.
        auto trigger = new QToolButton(controls);
        trigger->setText(t("settings"));
        trigger->setPopupMode(QToolButton::InstantPopup);
        auto menu = new QMenu(trigger);
        trigger->setMenu(menu);
        layout->addWidget(trigger);
        auto action = [this, menu](const char* key, std::function<void()> fn) {
            auto entry = menu->addAction(t(key));
            entry->setObjectName(key);
            connect(entry, &QAction::triggered, this, [this, fn] { run(fn); });
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
            action("playlistDetails", [this, record] { showLiveDetails(record); });
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
        table->setCellWidget(row, table->columnCount() - 1, controls);
        if (oldId.toLongLong() == id)
            table->selectRow(row);
    }
    if (cleanupLabel) {
        auto progress = state.get("cleanup");
        cleanupLabel->setText(
            t(progress.get("running").flag() ? "cleanupInProgress" : "cleanupSuccess")
                .replace("{0}", QLocale().toString(progress.get("deletedLives").num())));
        cleanupBar->setMaximum(std::max(1, int(progress.get("totalVideos").num())));
        cleanupBar->setValue(progress.get("deletedVideos").num());
    }
}

// I controlli di regia operano sulla sessione scelta, mentre il frame viene letto direttamente dal
// disco.
void NativeWindow::buildPreview() {
    livePicker = new QComboBox(body);
    livePicker->setObjectName("livePicker");
    bodyLayout->addWidget(livePicker);
    preview = new QLabel(t("previewWaiting"), body);
    preview->setObjectName("livePreview");
    preview->setMinimumSize(640, 360);
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
    addAction("playSpot", [this] {
        const auto file = QFileDialog::getOpenFileName(this, t("spotFile"));
        if (!file.isEmpty()) {
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
            liveCommand(watched, "spot", parameters);
        }
    });
    addAction("next", [this] { liveCommand(watched, "next"); });
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
    if (!watched) {
        preview->clear();
        preview->setText(t("noLiveRunning"));
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
    auto label = body->findChild<QLabel*>("metrics");
    if (!label) {
        label = new QLabel(body);
        label->setObjectName("metrics");
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        bodyLayout->addWidget(label);
    }
    auto values = state.get("metrics"), process = state.get("process");
    QStringList lines;
    lines.append(t("cpu") + ": " + formatMetric(values.get("cpu").num(-1), "%"));
    lines.append(t("ramUsed") + ": " + formatMetric(values.get("ram").num(-1), "%"));
    lines.append("SWAP: " + formatMetric(values.get("swap").num(-1), "%"));
    lines.append("↓ " + formatMetric(values.get("networkRx").num(-1) / 1024.0, " KiB/s") + " · ↑ " +
                 formatMetric(values.get("networkTx").num(-1) / 1024.0, " KiB/s"));
    lines.append(t("diskFree") + ": " +
                 formatMetric(values.get("diskAvailable").num(-1) / 1073741824.0, " GiB"));
    lines.append(t("uptime") + ": " + formatMetric(values.get("uptime").num(-1), " s"));
    lines.append(t("appUsage") + ": " + formatMetric(process.get("cpu").num(-1), "%") + " · " +
                 formatMetric(process.get("rssBytes").num(-1) / 1048576.0, " MiB"));
    auto temperatures = values.get("temperatures");
    for (size_t i = 0; i < temperatures.size(); ++i)
        lines.append(s(temperatures.at(i).get("label")) + ": " +
                     formatMetric(temperatures.at(i).get("celsius").num(-1), " °C"));
    auto live = state.get("live");
    for (size_t i = 0; i < live.size(); ++i)
        for (auto key : {"producerMetrics", "publisherMetrics"}) {
            auto metrics = live.at(i).get(key);
            if (metrics.has("pid"))
                lines.append("FFmpeg · PID " + QString::number(metrics.get("pid").num()) + " · " +
                             formatMetric(metrics.get("cpu").num(-1), "%") + " · " +
                             formatMetric(metrics.get("rssBytes").num(-1) / 1048576.0, " MiB"));
        }
    label->setText(lines.join('\n'));
}

// Le impostazioni aprono sezioni e dialoghi interni, conservando un’unica finestra applicativa.
void NativeWindow::buildSettings() {
    addAction("channelSettings", [this] { navigate("channels"); });
    addAction("videoSettings", [this] { navigate("profiles"); });
    addAction("sceneDeck", [this] { navigate("buttons"); });
    addAction("overlays", [this] { navigate("overlays"); });
    addAction("sceneMedia", [this] { navigate("scene-media"); });
    addAction("save", [this] { exportData(); });
    addAction("loadVideoAndAudioSettings", [this] { importData(); });
    addAction("about", [this] {
        QMessageBox::information(
            this, t("about"),
            QString("OrbisStream %1\nC++ · Qt 6\n%2").arg(ORBIS_VERSION, t("aboutVersion")));
    });
}

// L'avvio standard crea QApplication; l'API legacy resta solo nella modalità headless esplicita.
int runNativeApp(int /*argc*/, char** argv, Studio& studio, const fs::path& resources) {
    // Le opzioni del motore sono già state lette; Qt riceve solo il nome eseguibile.
    int qtArgc = 1;
    QApplication app(qtArgc, argv);
    app.setApplicationName("OrbisStream");
    app.setOrganizationName("OrbisStream");
    NativeWindow window(studio, resources);
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
    studio.stopAll();
    return result;
}
} // namespace orbis

#include "native_forms.hpp"
#include "native_theme.hpp"
#include "native_window.hpp"
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QGraphicsView>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <QTableWidget>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QToolButton>
#include <QVideoFrame>
#include <QVideoSink>
#include <QVideoWidget>
#include <QWizard>

using namespace orbis;

// I test attraversano widget, cambi lingua e dialoghi reali, senza il server HTTP.
class NativeTests : public QObject {
    Q_OBJECT
    QUrl openedUrl;
  private slots:

    void captureUrl(const QUrl& url) {
        openedUrl = url;
    }

    void dashboardMatchesWindows() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        {
            NativeWindow window(studio, ORBIS_TEST_RESOURCES);
            window.setLanguage("en");
            window.show();
            for (int width : {960, 1200, 1500}) {
                window.resize(width, 1000);
                QApplication::processEvents();
                const auto grids = window.findChildren<QWidget*>("dashboardGrid");
                QCOMPARE(grids.size(), 4);
                const QList<int> counts = {4, 2, 1, 1};
                for (int index = 0; index < grids.size(); ++index) {
                    auto grid = grids[index];
                    auto cards =
                        grid->findChildren<QPushButton*>(QString(), Qt::FindDirectChildrenOnly);
                    QCOMPARE(cards.size(), counts[index]);
                    for (auto card : cards) {
                        QVERIFY(grid->rect().contains(card->geometry()));
                        for (auto label : card->findChildren<QLabel*>())
                            QVERIFY2(card->rect().contains(label->geometry()),
                                     qPrintable(label->text()));
                    }
                }
                auto live = window.findChild<QPushButton*>("liveManagement");
                auto history = window.findChild<QPushButton*>("liveHistory");
                QCOMPARE(history->y() == live->y(), width == 1500);
            }
            window.resize(1380, 1000);
            QApplication::processEvents();
            QVERIFY(window.grab().save(
                QString(ORBIS_TEST_SCREENSHOT).replace("native-window", "native-dashboard-dark")));
            QVERIFY(!window.findChild<QLabel*>("metricValue"));
            QDesktopServices::setUrlHandler("https", this, "captureUrl");
            window.findChild<QPushButton*>("coffeeBanner")->click();
            QDesktopServices::unsetUrlHandler("https");
            QCOMPARE(openedUrl, QUrl("https://buymeacoffee.com/zaccalos"));
            const QList<QPair<QString, QString>> routes = {
                {"liveManagement", "Live Stream Management"},
                {"preview", "Preview"},
                {"layouts", "Layouts"},
                {"liveHistory", "Live History"},
                {"videoAndAudio", "Video settings"},
                {"channelSettings", "Channel Settings"},
                {"taskManager", "Task Manager"},
                {"manualTitle", "User Guide"}};
            NativeTranslations text(ORBIS_TEST_RESOURCES "/ui");
            text.setLanguage("en");
            const QStringList titleKeys = {"liveManagement", "preview",       "layouts",
                                           "liveHistory",    "videoSettings", "channelSettings",
                                           "taskManager",    "manual"};
            for (int i = 0; i < routes.size(); ++i) {
                window.navigate("dashboard");
                window.findChild<QPushButton*>(routes[i].first)->click();
                QCOMPARE(window.findChild<QLabel*>("pageTitle")->text(),
                         text.text(titleKeys[i].toUtf8().constData()));
            }
            window.navigate("dashboard");
            window.findChild<QToolButton*>("theme_light")->click();
            QCOMPARE(qApp->palette().color(QPalette::Window), QColor("#f4f4f7"));
            QVERIFY(window.findChild<QToolButton*>("theme_light")->isChecked());
            QVERIFY(!window.findChild<QToolButton*>("theme_dark")->isChecked());
            QApplication::processEvents();
            QVERIFY(window.grab().save(
                QString(ORBIS_TEST_SCREENSHOT).replace("native-window", "native-dashboard-light")));
            window.setLanguage("ar");
            QApplication::processEvents();
            auto first = window.findChild<QPushButton*>("liveManagement");
            auto second = window.findChild<QPushButton*>("preview");
            QVERIFY(first->x() > second->x());
        }
        NativeWindow reopened(studio, ORBIS_TEST_RESOURCES);
        QVERIFY(reopened.findChild<QToolButton*>("theme_light")->isChecked());
        QCOMPARE(qApp->property("orbisTheme").toString(), QString("light"));
        reopened.setTheme("dark");
        QCOMPARE(qApp->palette().color(QPalette::Window), QColor("#1a1a1c"));
    }

    void catalogsAndNativeNavigation() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        NativeTranslations catalogs(ORBIS_TEST_RESOURCES "/ui");
        const auto keys = catalogs.keys();
        QCOMPARE(catalogs.languages().size(), 21);
        NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        window.show();
        for (const auto& language : catalogs.languages()) {
            catalogs.setLanguage(language);
            QCOMPARE(catalogs.keys(), keys);
            for (const auto& key : keys)
                QVERIFY(!catalogs.text(key.toUtf8().constData()).isEmpty());
            window.setLanguage(language);
            QCOMPARE(window.language(), language);
            QCOMPARE(qApp->layoutDirection(), language == "ar" ? Qt::RightToLeft : Qt::LeftToRight);
            for (auto page : {"dashboard", "channels", "profiles", "live", "history", "layouts",
                              "preview", "playlist", "buttons", "overlays", "scene-media",
                              "activity", "manual", "settings"}) {
                window.navigate(page);
                QApplication::processEvents();
                QTRY_VERIFY_WITH_TIMEOUT(window.findChild<QLabel*>("pageTitle")->isVisible(), 1000);
            }
        }
        window.setLanguage("it");
        window.navigate("dashboard");
        QCOMPARE(window.findChild<QLabel*>("pageTitle")->text(), QString("Pannello"));
        QVERIFY(window.findChild<QComboBox*>("languagePicker"));
        QVERIFY(window.findChildren<QWidget*>().size() > 10);
        QTRY_VERIFY_WITH_TIMEOUT(window.findChild<QLabel*>("pageTitle")->isVisible(), 1000);
        QVERIFY(window.grab().save(ORBIS_TEST_SCREENSHOT));
        auto preferences = studio.store.get("preferences", 1);
        QCOMPARE(preferences.get("language").str(), std::string("it"));
    }

    // Le schede devono restare intere e i comandi senza sessione non devono inviare richieste.
    void readableCardsAndEmptyPreview() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        window.setLanguage("it");
        window.resize(960, 640);
        window.show();
        QApplication::processEvents();
        for (auto card : window.findChildren<QPushButton*>()) {
            if (!card->property("card").toBool())
                continue;
            QVERIFY(card->parentWidget()->rect().contains(card->geometry()));
            for (auto label : card->findChildren<QLabel*>())
                QVERIFY(card->rect().contains(label->geometry()));
        }
        window.navigate("preview");
        QApplication::processEvents();
        auto stop = window.findChild<QPushButton*>("stopLiveIconLabel");
        QVERIFY(stop);
        QVERIFY(!stop->isEnabled());
        QVERIFY(window.findChild<QPushButton*>("startLiveButton")->isEnabled());
        window.navigate("live");
        QApplication::processEvents();
        QVERIFY(window.findChild<QLabel*>("emptyState")->isVisible());
        QVERIFY(!window.findChild<QTableWidget*>("recordsTable")->isVisible());
        window.resize(1380, 900);
        QApplication::processEvents();
        QVERIFY(window.grab().save(
            QString(ORBIS_TEST_SCREENSHOT).replace("native-window", "native-live")));
    }

    void nativeChannelAndProfileForms() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        window.setLanguage("en");
        window.show();
        window.navigate("channels");
        auto add = window.findChild<QPushButton*>("addConfiguration");
        QVERIFY(add);
        QTimer::singleShot(0, &window, [&] {
            auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto name = dialog->findChild<QLineEdit*>("channelName");
            QVERIFY(name);
            name->setText("Native test");
            dialog->findChild<QLineEdit*>("streamUrl")->setText("rtmp://127.0.0.1/live");
            dialog->findChild<QCheckBox*>("isActive")->setChecked(true);
            QTest::mouseClick(
                dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save),
                Qt::LeftButton);
        });
        QTest::mouseClick(add, Qt::LeftButton);
        QCOMPARE(studio.store.list("channels").size(), size_t(1));
        QCOMPARE(studio.store.list("channels").at(0).get("channelName").str(),
                 std::string("Native test"));
        window.navigate("profiles");
        auto profile = window.findChild<QPushButton*>("addNewAudioVideoConfiguration");
        QVERIFY(profile);
        QTimer::singleShot(0, &window, [&] {
            auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            dialog->findChild<QLineEdit*>("title")->setText("Native profile");
            dialog->findChild<QCheckBox*>("isActive")->setChecked(true);
            QTest::mouseClick(
                dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save),
                Qt::LeftButton);
        });
        QTest::mouseClick(profile, Qt::LeftButton);
        auto profiles = studio.store.list("profiles");
        bool found = false;
        for (size_t i = 0; i < profiles.size(); ++i)
            if (profiles.at(i).get("title").str() == "Native profile")
                found = true;
        QVERIFY(found);
    }

    // Parte dal wizard Qt e controlla che FFmpeg riceva realmente la playlist su localhost.
    void playlistWizardStreams() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        auto root = fs::path(directory.path().toStdString());
        auto video = root / "sample.mp4";
        capture({"ffmpeg",      "-v",        "error", "-y",
                 "-f",          "lavfi",     "-i",    "color=c=blue:s=320x180:r=15",
                 "-f",          "lavfi",     "-i",    "sine=frequency=440",
                 "-t",          "8",         "-c:v",  "libx264",
                 "-preset",     "ultrafast", "-c:a",  "aac",
                 video.string()},
                15);
        QTcpServer listener;
        QVERIFY(listener.listen(QHostAddress::LocalHost, 0));
        auto port = listener.serverPort();
        listener.close();
        auto target = "rtmp://127.0.0.1:" + std::to_string(port) + "/live";
        Process ingest;
        auto recorded = root / "received.flv";
        ingest.start({"ffmpeg", "-v", "error", "-y", "-listen", "1", "-i", target + "/test-key",
                      "-c", "copy", "-f", "flv", recorded.string()},
                     -1, root / "ingest.log", false);
        QTest::qWait(250);
        Json profile;
        profile.set("title", "Native streaming");
        profile.set("isActive", true);
        profile.set("videoWidth", 320.0);
        profile.set("videoHeight", 180.0);
        profile.set("frameRate", 15.0);
        profile.set("videoBitrate", 500.0);
        profile.set("audioBitrate", 64.0);
        profile.set("preset", "ultrafast");
        profile.set("encoderQuality", "light");
        studio.store.save("profiles", profile);
        Json channel;
        channel.set("channelName", "Qt RTMP");
        channel.set("platform", "Custom");
        channel.set("streamUrl", target);
        channel.set("streamKey", "test-key");
        channel.set("isActive", true);
        studio.store.save("channels", channel);
        NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        window.setLanguage("en");
        window.show();
        window.navigate("live");
        QTimer::singleShot(0, &window, [&] {
            auto wizard = qobject_cast<QWizard*>(QApplication::activeModalWidget());
            QVERIFY(wizard);
            auto profiles = wizard->findChild<QListWidget*>("wizardProfile");
            QVERIFY(profiles);
            QVERIFY(profiles->currentItem());
            QVERIFY(profiles->currentItem()->text().contains("kbps"));
            QVERIFY(wizard->grab().save(
                QString(ORBIS_TEST_SCREENSHOT).replace("native-window", "native-wizard")));
            wizard->next();
            wizard->next();
            wizard->findChild<QLineEdit*>("wizardFolder")
                ->setText(QString::fromStdString(root.string()));
            QTest::mouseClick(wizard->button(QWizard::FinishButton), Qt::LeftButton);
        });
        QTest::mouseClick(window.findChild<QPushButton*>("startPlaylistButton"), Qt::LeftButton);
        QTRY_VERIFY_WITH_TIMEOUT(studio.live().size() == 1, 15000);
        QTRY_VERIFY_WITH_TIMEOUT(fs::exists(recorded) && fs::file_size(recorded) > 10000, 15000);
        auto id = studio.live().at(0).get("id").num();
        QTRY_VERIFY_WITH_TIMEOUT(
            fs::exists(studio.store.root / "previews" / (std::to_string(long(id)) + ".jpg")), 8000);
        studio.stopAll();
        ingest.stop();
        QVERIFY(probe(recorded.string()).get("format").get("duration").num() > 0);
    }

    // Il player viene aperto dall'azione della libreria; deve decodificare frame reali nel widget
    // Qt.
    void nativePlayerDecodesFrames() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        auto root = fs::path(directory.path().toStdString());
        auto file = root / "preview.mp4";
        capture({"ffmpeg", "-v", "error", "-y", "-f", "lavfi", "-i", "color=c=green:s=320x180:r=15",
                 "-t", "4", "-c:v", "libx264", "-preset", "ultrafast", file.string()},
                10);
        Json video;
        video.set("name", "Native player");
        video.set("videoPath", file.string());
        studio.store.save("videos", video);
        NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        window.show();
        window.navigate("playlist");
        auto table = window.findChild<QTableWidget*>("recordsTable");
        QVERIFY(table);
        QTRY_COMPARE_WITH_TIMEOUT(table->rowCount(), 1, 5000);
        int frames = 0;
        QString failure;
        QTimer::singleShot(0, &window, [&] {
            auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto player = dialog->findChild<QMediaPlayer*>();
            auto output = dialog->findChild<QVideoWidget*>();
            QVERIFY(player);
            QVERIFY(output);
            connect(output->videoSink(), &QVideoSink::videoFrameChanged, dialog,
                    [&](const QVideoFrame& frame) {
                        if (frame.isValid())
                            ++frames;
                    });
            connect(player, &QMediaPlayer::errorOccurred, dialog,
                    [&](QMediaPlayer::Error, const QString& message) { failure = message; });
            QTimer::singleShot(2500, dialog, &QDialog::accept);
        });
        auto preview = window.findChild<QAction*>("preview");
        QVERIFY(preview);
        preview->trigger();
        QVERIFY2(failure.isEmpty(), qPrintable(failure));
        QVERIFY(frames > 1);
    }

    void layoutCanvasPersistsGeometry() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        window.setLanguage("en");
        window.show();
        window.navigate("layouts");
        QApplication::processEvents();
        QVERIFY(window.findChild<QFrame*>("layoutEmpty")->isVisible());
        QVERIFY(!window.findChild<QTableWidget*>("recordsTable"));
        QVERIFY(!window.findChild<QPushButton*>("sceneStrip"));
        QVERIFY(!window.findChild<QPushButton*>("overlays"));
        auto create = window.findChild<QPushButton*>("newLayout");
        QVERIFY(create);
        QVERIFY(window.findChild<QFrame*>("pageHeading")->isAncestorOf(create));
        QTimer::singleShot(0, &window, [&] {
            auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto canvas = dialog->findChild<QGraphicsView*>("sceneCanvas");
            QVERIFY(canvas);
            QTimer::singleShot(10000, dialog, &QDialog::reject);
            auto commit = dialog->findChild<QPushButton*>("commitScene");
            QVERIFY(commit);
            QCOMPARE(commit->text(), QString("Save layout"));
            QTest::mouseClick(dialog->findChild<QPushButton*>("addSlot"), Qt::LeftButton);
            QApplication::processEvents();
            auto layers = dialog->findChild<QListWidget*>("sceneLayers");
            QVERIFY(layers);
            QTRY_COMPARE(layers->count(), 1);
            dialog->findChild<QDoubleSpinBox*>("source_width")->setValue(640);
            dialog->findChild<QDoubleSpinBox*>("source_height")->setValue(360);
            dialog->findChild<QDoubleSpinBox*>("source_x")->setValue(30);
            dialog->findChild<QDoubleSpinBox*>("source_y")->setValue(20);
            QVERIFY(dialog->grab().save(
                QString(ORBIS_TEST_SCREENSHOT).replace("native-window", "native-composer")));
            QTest::mouseClick(commit, Qt::LeftButton);
        });
        QTest::mouseClick(create, Qt::LeftButton);
        auto saved = studio.store.list("scenes");
        QCOMPARE(saved.size(), size_t(1));
        auto tile = saved.at(0).get("items").at(0);
        QCOMPARE(tile.get("x").num(), 30.0);
        QCOMPARE(tile.get("y").num(), 20.0);
        QCOMPARE(tile.get("width").num(), 640.0);
        QCOMPARE(tile.get("height").num(), 360.0);
        QVERIFY(saved.at(0).get("isLayout").flag());
        QApplication::processEvents();
        QTRY_VERIFY(window.findChild<QFrame*>("layoutRow"));
        auto row = window.findChild<QFrame*>("layoutRow");
        QVERIFY(row);
        QVERIFY(row->findChild<QToolButton*>("seeDetails"));
        QVERIFY(row->findChild<QToolButton*>("delete"));
        QTimer::singleShot(0, &window, [&] {
            auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            QVERIFY(dialog->findChild<QGraphicsView*>("sceneCanvas"));
            dialog->reject();
        });
        QTest::mouseClick(row->findChild<QToolButton*>("seeDetails"), Qt::LeftButton);
        QVERIFY(window.grab().save(
            QString(ORBIS_TEST_SCREENSHOT).replace("native-window", "native-layouts")));
        QTimer::singleShot(0, &window, [&] {
            auto confirmation = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
            QVERIFY(confirmation);
            confirmation->button(QMessageBox::Yes)->click();
        });
        QTest::mouseClick(row->findChild<QToolButton*>("delete"), Qt::LeftButton);
        QCOMPARE(studio.store.list("scenes").size(), size_t(0));
        QApplication::processEvents();
        QTRY_VERIFY(window.findChild<QFrame*>("layoutEmpty"));
        QTRY_VERIFY(window.findChild<QFrame*>("layoutEmpty")->isVisible());
        QTest::mouseClick(window.findChild<QToolButton*>("layoutBack"), Qt::LeftButton);
        QVERIFY(window.findChild<QPushButton*>("startLiveButton"));
    }

    // La pagina Gestione Live segue MainLive: intestazioni, pillole e azioni come icone sempre
    // visibili, con il marchio della piattaforma di ogni riga.
    void liveRowsMatchMainLiveLayout() {
        QTemporaryDir directory;
        Studio studio(directory.path().toStdString());
        Json profile;
        profile.set("id", 1.0);
        profile.set("title", "1080p");
        profile.set("videoWidth", 1920.0);
        profile.set("videoHeight", 1080.0);
        profile.set("isActive", true);
        studio.store.save("profiles", profile, 1);
        auto seed = [&](const char* channelName, const char* platform, const char* url) {
            Json channel;
            channel.set("channelName", channelName);
            channel.set("platform", platform);
            channel.set("streamUrl", url);
            channel.set("profileId", 1.0);
            channel.set("isActive", true);
            studio.store.save("channels", channel);
        };
        seed("Marco Live", "Twitch", "rtmp://live.twitch.tv/app");
        seed("OrbisTube", "YouTube", "rtmp://a.rtmp.youtube.com/live2");
        seed("KickChannel", "Kick", "rtmp://fa723fc1b171.global-contribute.live-video.net/app");
        auto push = [&](const Json& request, const char* status, double position,
                        const char* channelName, const char* platform, const char* streamUrl) {
            Json h;
            h.set("status", status);
            h.set("startedAt", std::string("2026-10-14T10:20:30"));
            h.set("channelId", 1.0);
            h.set("channelName", channelName);
            h.set("platform", platform);
            h.set("position", position);
            auto r = request.clone();
            r.set("profileId", 1.0);
            r.set("channelName", channelName);
            r.set("platform", platform);
            r.set("streamUrl", streamUrl);
            h.set("request", r);
            studio.store.save("history", h);
        };
        auto files = [](std::initializer_list<const char*> paths) {
            Json array = Json::array();
            for (const char* path : paths)
                array.push(Json::string(path));
            Json request;
            request.set("files", array);
            return request;
        };
        push(files({"/videos/solo/single-live.mp4"}), "LIVE", 0, "Marco Live", "Twitch",
             "rtmp://live.twitch.tv/app");
        push(files({"/videos/show/ep1.mp4", "/videos/show/ep2.mp4", "/videos/show/ep3.mp4"}),
             "ENDED", 1, "OrbisTube", "YouTube", "rtmp://a.rtmp.youtube.com/live2");
        Json sceneRequest;
        Json scene;
        scene.set("name", "Studio scene");
        Json items = Json::array();
        for (int i = 0; i < 4; ++i) {
            Json item;
            item.set("sourceKind", "FILE");
            item.set("sourceTarget", std::string("/videos/scene/src") + std::to_string(i) + ".mp4");
            items.push(item);
        }
        scene.set("items", items);
        sceneRequest.set("scene", scene);
        push(sceneRequest, "STARTING", 0, "KickChannel", "Kick",
             "rtmp://fa723fc1b171.global-contribute.live-video.net/app");

        NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        window.setLanguage("en");
        window.resize(1380, 800);
        window.show();
        window.navigate("live");
        auto table = window.findChild<QTableWidget*>("recordsTable");
        QVERIFY(table);
        QTRY_COMPARE_WITH_TIMEOUT(table->rowCount(), 3, 5000);
        // Le intestazioni sono quelle di MainLive, con la data senza i due punti finali.
        QCOMPARE(table->columnCount(), 5);
        QStringList headers;
        for (int c = 0; c < table->columnCount(); ++c)
            headers << table->horizontalHeaderItem(c)->text();
        QCOMPARE(headers, QStringList({"Live Status", "Title", "Channel", "Start Date", ""}));
        // Il marchio di ogni piattaforma nota si disegna davvero (percorsi SVG, non icone Qt).
        for (auto name : {"twitch", "youtube", "kick", "facebook", "tiktok"}) {
            QImage mark(24, 24, QImage::Format_ARGB32);
            mark.fill(Qt::transparent);
            QPainter painter(&mark);
            platformMark(QString(name)).paint(&painter, 0, 0, 24, 24);
            painter.end();
            int filled = 0;
            for (int y = 0; y < 24; ++y)
                for (int x = 0; x < 24; ++x)
                    if (qAlpha(mark.pixel(x, y)) > 0)
                        ++filled;
            QVERIFY2(filled > 100, qPrintable(QString("%1 mark empty").arg(name)));
        }
        auto rowFor = [&](const QString& channel) {
            for (int r = 0; r < table->rowCount(); ++r)
                if (table->item(r, 2)->text() == channel)
                    return r;
            return -1;
        };
        auto actionTooltips = [&](int row) {
            QStringList text;
            for (auto button : table->cellWidget(row, 4)->findChildren<QToolButton*>())
                text << button->toolTip();
            return text;
        };
        // La playlist di YouTube: pillola della posizione, cartella come provenienza, marchio suo.
        auto playlist = rowFor("OrbisTube");
        QVERIFY(playlist >= 0);
        QCOMPARE(table->cellWidget(playlist, 1)->findChild<QLabel*>("rowTitle")->text(),
                 QString("ep2.mp4"));
        QCOMPARE(table->cellWidget(playlist, 1)->findChild<QLabel*>("rowCaption")->text(),
                 QString("YouTube · /videos/show"));
        QCOMPARE(actionTooltips(playlist),
                 QStringList({"YouTube", "Playlist details", "Connect Video Setting",
                              "Scene switch", "Delete playlist"}));
        // La scena di Kick: il gettone delle sorgenti e la provenienza della prima sorgente.
        auto sceneRow = rowFor("KickChannel");
        QVERIFY(sceneRow >= 0);
        QCOMPARE(table->cellWidget(sceneRow, 1)->findChild<QLabel*>("rowTitle")->text(),
                 QString("Studio scene"));
        QCOMPARE(table->cellWidget(sceneRow, 1)->findChild<QLabel*>("rowCaption")->text(),
                 QString("Kick · src0.mp4"));
        QCOMPARE(actionTooltips(sceneRow),
                 QStringList({"Kick", "Scene details", "Connect Video Setting", "Scene switch",
                              "Play Ad Spot", "Stop Live", "Start Live", "Restart Live",
                              "Delete playlist"}));
        // La diretta singola senza sessione: solo l'avvio è disponibile, stop e spot sono spenti.
        auto single = rowFor("Marco Live");
        QVERIFY(single >= 0);
        QCOMPARE(actionTooltips(single),
                 QStringList({"Twitch", "Connect Video Setting", "Scene switch", "Play Ad Spot",
                              "Stop Live", "Start Live", "Restart Live", "Delete row"}));
        auto actions = table->cellWidget(single, 4)->findChildren<QToolButton*>();
        QCOMPARE(actions.size(), 8);
        QVERIFY(!actions.at(3)->isEnabled());
        QVERIFY(!actions.at(4)->isEnabled());
        QVERIFY(actions.at(5)->isEnabled());
        QVERIFY(window.grab().save(
            QString(ORBIS_TEST_SCREENSHOT).replace("native-window", "native-live-populated")));
    }
};
QTEST_MAIN(NativeTests)
#include "native_tests.moc"

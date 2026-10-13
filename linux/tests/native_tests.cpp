#include "native_forms.hpp"
#include "native_window.hpp"
#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QGraphicsView>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlayer>
#include <QTableWidget>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QVideoFrame>
#include <QVideoSink>
#include <QVideoWidget>
#include <QWizard>

using namespace orbis;

// I test attraversano widget, cambi lingua e dialoghi reali, senza il server HTTP.
class NativeTests : public QObject {
    Q_OBJECT
  private slots:

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
        Json layout;
        layout.set("isLayout", true);
        layout.set("name", "Qt layout");
        QTimer::singleShot(0, &window, [&] {
            auto dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
            QVERIFY(dialog);
            auto canvas = dialog->findChild<QGraphicsView*>("sceneCanvas");
            QVERIFY(canvas);
            QTest::mouseClick(dialog->findChild<QPushButton*>("addSlot"), Qt::LeftButton);
            dialog->findChild<QDoubleSpinBox*>("source_width")->setValue(640);
            dialog->findChild<QDoubleSpinBox*>("source_height")->setValue(360);
            dialog->findChild<QDoubleSpinBox*>("source_x")->setValue(30);
            dialog->findChild<QDoubleSpinBox*>("source_y")->setValue(20);
            QTest::mouseClick(
                dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save),
                Qt::LeftButton);
        });
        window.editScene(layout);
        auto saved = studio.store.list("scenes");
        QCOMPARE(saved.size(), size_t(1));
        auto tile = saved.at(0).get("items").at(0);
        QCOMPARE(tile.get("x").num(), 30.0);
        QCOMPARE(tile.get("y").num(), 20.0);
        QCOMPARE(tile.get("width").num(), 640.0);
        QCOMPARE(tile.get("height").num(), 360.0);
        QVERIFY(saved.at(0).get("isLayout").flag());
    }
};
QTEST_MAIN(NativeTests)
#include "native_tests.moc"

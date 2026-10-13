#include "native_window.hpp"
#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QTimer>
#include <cerrno>
#include <iostream>
#include <signal.h>

// La chiusura della regia deve uscire anche con una seconda finestra e una diretta attiva.
int main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    QApplication app(argc, argv);
    QTemporaryDir directory;
    orbis::Studio studio(directory.path().toStdString());
    auto video = studio.store.root / "sample.mp4";
    orbis::capture({"ffmpeg", "-v", "error", "-y", "-f", "lavfi", "-i",
                    "color=c=blue:s=320x180:r=15", "-t", "4", "-c:v", "libx264", "-preset",
                    "ultrafast", video.string()},
                   10);
    auto request = orbis::Json::parse(R"({"profile":{"title":"Shutdown test",
        "videoWidth":320,"videoHeight":180,"frameRate":15,"videoBitrate":500,
        "audioBitrate":64,"preset":"ultrafast"},"loop":true})");
    auto files = orbis::Json::array();
    files.push(orbis::Json::string(video.string()));
    request.set("files", files);
    request.set("outputFile", (studio.store.root / "output.flv").string());
    auto id = studio.start(request);
    long producer = 0, publisher = 0;
    int result = 1;
    {
        orbis::NativeWindow window(studio, ORBIS_TEST_RESOURCES);
        QDialog secondary;
        secondary.show();
        window.show();
        QTimer closeTimer;
        QObject::connect(&closeTimer, &QTimer::timeout, &window, [&] {
            auto state = studio.session(id)->snapshot();
            if (state.get("status").str() == "ERROR") {
                std::cerr << state.dump() << '\n';
                app.exit(3);
                return;
            }
            producer = state.get("producerPid").num();
            publisher = state.get("publisherPid").num();
            if (producer <= 0 || publisher <= 0 || state.get("outputKbps").num() <= 0)
                return;
            closeTimer.stop();
            QTimer::singleShot(0, &window, [] {
                if (auto box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget()))
                    box->button(QMessageBox::Yes)->click();
            });
            window.close();
        });
        closeTimer.start(100);
        QTimer::singleShot(15000, &window, [&] { app.exit(2); });
        result = app.exec();
    }
    auto gone = [](long pid) { return pid > 0 && kill(pid, 0) == -1 && errno == ESRCH; };
    if (result || !studio.session(id)->done() || !gone(producer) || !gone(publisher)) {
        std::cerr << "Main window close left the event loop or streaming children running: "
                  << result << " " << studio.session(id)->snapshot().dump() << '\n';
        return 1;
    }
    std::cout << "Main window close exits with secondary window and stops FFmpeg: OK\n";
}

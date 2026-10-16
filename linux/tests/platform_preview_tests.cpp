#include "core.hpp"
#include "platform_preview.hpp"
#include <QApplication>
#include <QFile>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QWebEnginePage>
#include <QtTest>

using namespace orbis;

class ObservedPage : public QWebEnginePage {
  public:
    double played = 0;

    explicit ObservedPage(QObject* parent) : QWebEnginePage(parent) {}

  protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel, const QString& message, int,
                                  const QString&) override {
        if (message.startsWith("played:"))
            played = message.mid(7).toDouble();
    }
};

class PlatformPreviewTests : public QObject {
    Q_OBJECT
  private slots:

    void embeddedPlayerDecodesVideo() {
        configureWebEngineRuntime();
        QTemporaryDir directory;
        const auto file = directory.path() + "/video.mp4";
        capture({ffmpeg(), "-v", "error", "-y", "-f", "lavfi", "-i", "color=c=blue:s=320x180:r=15",
                 "-t", "3", "-c:v", "libx264", "-pix_fmt", "yuv420p", "-movflags", "+faststart",
                 file.toStdString()});
        QFile video(file);
        QVERIFY(video.open(QIODevice::ReadOnly));
        const auto data = video.readAll();
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost, 0));
        connect(&server, &QTcpServer::newConnection, this, [&] {
            while (auto socket = server.nextPendingConnection()) {
                connect(socket, &QTcpSocket::readyRead, socket, [socket, data] {
                    auto request = socket->property("request").toByteArray() + socket->readAll();
                    socket->setProperty("request", request);
                    if (!request.contains("\r\n\r\n") || socket->property("sent").toBool())
                        return;
                    socket->setProperty("sent", true);
                    QByteArray body, header;
                    if (request.startsWith("GET /video.mp4")) {
                        int start = 0;
                        for (const auto& line : request.split('\n'))
                            if (line.toLower().startsWith("range: bytes="))
                                start = line.mid(13).split('-').first().toInt();
                        start = std::clamp(start, 0, int(data.size()));
                        body = data.mid(start);
                        header = "HTTP/1.1 206 Partial Content\r\nContent-Type: "
                                 "video/mp4\r\nAccept-Ranges: bytes\r\nContent-Range: bytes " +
                                 QByteArray::number(start) + "-" +
                                 QByteArray::number(data.size() - 1) + "/" +
                                 QByteArray::number(data.size()) + "\r\n";
                    } else {
                        body = "<!doctype html><body style='margin:0;background:red'><video "
                               "autoplay muted loop playsinline style='width:100%;height:100%' "
                               "src='/video.mp4' "
                               "ontimeupdate=\"console.log('played:'+this.currentTime)\"></video>";
                        header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";
                    }
                    socket->write(header + "Content-Length: " + QByteArray::number(body.size()) +
                                  "\r\nConnection: close\r\n\r\n" + body);
                    socket->disconnectFromHost();
                });
                connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            }
        });
        PlatformPreview preview(nullptr);
        preview.resize(640, 360);
        auto page = new ObservedPage(&preview);
        preview.setPage(page);
        QSignalSpy state(&preview, &PlatformPreview::playerState);
        preview.loadEmbed(QUrl(QString("http://127.0.0.1:%1/player").arg(server.serverPort())));
        QTRY_VERIFY_WITH_TIMEOUT(page->played > .3, 15000);
        QVERIFY(preview.isVisible());
        QVERIFY(state.count() > 0);
        QVERIFY(preview.grab().save(directory.path() + "/embedded-player.png"));
        preview.stop();
        preview.hide();
    }
};
QTEST_MAIN(PlatformPreviewTests)
#include "platform_preview_tests.moc"

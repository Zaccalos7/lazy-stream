#include "platform_preview.hpp"
#include "media_job.hpp"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QWebEngineFullScreenRequest>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QtConcurrent>

namespace orbis {
void configureWebEngineRuntime() {
    // A developer build can use unprivileged, extracted distro packages. Installed builds use Qt's
    // paths.
    const auto root = fs::read_symlink("/proc/self/exe").parent_path() / "qt-webengine" / "usr";
    auto set = [&](const char* key, const fs::path& path) {
        if (!qEnvironmentVariableIsSet(key) && fs::exists(path))
            qputenv(key, QByteArray::fromStdString(path.string()));
    };
    set("QTWEBENGINEPROCESS_PATH", root / "lib/qt6/libexec/QtWebEngineProcess");
    set("QTWEBENGINE_RESOURCES_PATH", root / "share/qt6/resources");
    set("QTWEBENGINE_LOCALES_PATH", root / "share/qt6/translations/qtwebengine_locales");
    const auto libraries = root / "lib/x86_64-linux-gnu";
    if (fs::exists(libraries / "libQt6WebEngineCore.so.6")) {
        const auto prefix = QByteArray::fromStdString(libraries.string());
        const auto old = qgetenv("LD_LIBRARY_PATH");
        if (!old.split(':').contains(prefix))
            qputenv("LD_LIBRARY_PATH", prefix + (old.isEmpty() ? QByteArray{} : ":" + old));
    }
}

PlatformPreview::PlatformPreview(QWidget* parent) : QWebEngineView(parent) {
    setObjectName("platformPreview");
    setContextMenuPolicy(Qt::NoContextMenu);
    settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    page()->setBackgroundColor(Qt::black);
    connect(this, &QWebEngineView::loadStarted, this, [this] { emit playerState("LOADING"); });
    connect(this, &QWebEngineView::loadFinished, this, [this](bool ok) {
        setVisible(ok);
        emit playerState(ok ? "READY" : "FAILED");
    });
    connect(this, &QWebEngineView::renderProcessTerminated, this, [this] {
        hide();
        emit playerState("FAILED");
    });
    connect(page(), &QWebEnginePage::fullScreenRequested, this,
            [this](QWebEngineFullScreenRequest request) {
                request.accept();
                if (request.toggleOn())
                    window()->showFullScreen();
                else
                    window()->showNormal();
            });
    refresh.setInterval(30000);
    connect(&refresh, &QTimer::timeout, this, &PlatformPreview::resolve);
    hide();
}

void PlatformPreview::watch(const Json& value) {
    channel = value.clone();
    currentEmbed.clear();
    resolve();
    refresh.start();
}

void PlatformPreview::resolve() {
    if (querying)
        return;
    querying = true;
    auto watcher = new QFutureWatcher<Json>(this);
    connect(watcher, &QFutureWatcher<Json>::finished, this, [this, watcher] {
        querying = false;
        auto result = watcher->result();
        watcher->deleteLater();
        const auto embed = QString::fromStdString(result.get("embed").str());
        if (embed.isEmpty()) {
            hide();
            emit playerState(QString::fromStdString(result.get("air").str("UNAVAILABLE")));
            return;
        }
        if (embed != currentEmbed) {
            currentEmbed = embed;
            loadEmbed(QUrl(embed));
        }
    });
    const auto snapshot = channel.clone();
    watcher->setFuture(QtConcurrent::run([snapshot] {
        try {
            return platformPlayer(snapshot);
        } catch (...) {
            Json result;
            result.set("air", "UNVERIFIED");
            return result;
        }
    }));
}

void PlatformPreview::loadEmbed(const QUrl& url) {
    if (url.scheme() != "https" && url.scheme() != "http")
        return;
    // A localhost origin supplies Twitch's parent and YouTube's embed origin just as the Windows
    // shell.
    const auto source = url.toString(QUrl::FullyEncoded).toHtmlEscaped();
    setHtml("<!doctype html><html><head><meta name='viewport' "
            "content='width=device-width,initial-scale=1'></head><body "
            "style='margin:0;background:black;overflow:hidden'><iframe title='Live player' src='" +
                source +
                "' style='position:fixed;inset:0;width:100%;height:100%;border:0' "
                "allow='autoplay;fullscreen;picture-in-picture' "
                "allowfullscreen></iframe></body></html>",
            QUrl("http://localhost/"));
    show();
    raise();
}
} // namespace orbis

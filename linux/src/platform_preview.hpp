#pragma once
#include "core.hpp"
#include <QTimer>
#include <QWebEngineView>

namespace orbis {
// Only the public platform player uses WebEngine; all application controls remain C++ widgets.
class PlatformPreview : public QWebEngineView {
    Q_OBJECT
    Json channel;
    QString currentEmbed;
    QTimer refresh;
    bool querying = false;
    Qt::WindowStates previousWindowState;
    void resolve();

  public:
    explicit PlatformPreview(QWidget* parent);
    void watch(const Json& value);
    void loadEmbed(const QUrl& url);
  signals:
    void playerState(const QString& state);
    void diagnostic(const QString& message);
};

void configureWebEngineRuntime();
} // namespace orbis

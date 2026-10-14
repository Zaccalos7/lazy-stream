#pragma once
#include "core.hpp"
#include "media_job.hpp"
#include "native_i18n.hpp"
#include <QMainWindow>
#include <QPointer>
#include <functional>

class QComboBox;
class QLineEdit;
class QListWidget;
class QLabel;
class QTableWidget;
class QVBoxLayout;
class QTimer;
class QShortcut;
class QProgressBar;

namespace orbis {
class NativeWindow : public QMainWindow {
    Studio& studio;
    fs::path resources;
    NativeTranslations translations;
    ProxyLibrary proxies;
    Json state;
    QString page = "dashboard";
    QListWidget* navigation = nullptr;
    QComboBox* languagePicker = nullptr;
    QWidget* body = nullptr;
    QVBoxLayout* bodyLayout = nullptr;
    QTableWidget* table = nullptr;
    QLineEdit* search = nullptr;
    QComboBox *channelFilter = nullptr, *statusFilter = nullptr, *platformFilter = nullptr,
              *livePicker = nullptr;
    QLabel *preview = nullptr, *previewFacts = nullptr, *cleanupLabel = nullptr;
    QWidget *previewContent = nullptr, *previewMixer = nullptr, *previewDeck = nullptr;
    Json previewProfile;
    long previewControlsFor = 0;
    QTimer* previewParameterTimer = nullptr;
    QProgressBar* cleanupBar = nullptr;
    long watched = 0;
    long armedDeck = 0;
    QList<QShortcut*> deckShortcuts;
    int currentPage = 0, pageSize = 20;
    int liveActionsWidth = 0;
    QLabel* pageLabel = nullptr;
    QLabel* emptyState = nullptr;
    bool sampling = false;
    bool sidebarClosed = false;
    QStringList navigationKeys;
    QStringList navigationPages;
    QString t(const char* key) const;
    void rebuild();
    void refreshRows();
    void buildDashboard();
    void buildLiveMenu();
    void updateSidebar();
    void showAbout();
    void buildChannels();
    void buildProfiles();
    void buildLive(bool history);
    void refreshHistory();
    void unlockVideos();
    void applyPreviewParameters();
    void populatePreviewControls(const Json& snapshot);
    void buildScenes(bool layouts);
    void refreshLayouts();
    void buildPreview();
    void buildPlaylist();
    void buildMedia(const QString& kind);
    void buildButtons();
    void buildMetrics();
    void buildManual();
    void buildSettings();
    void editChannel(Json channel = Json{});
    void editProfile(Json profile = Json{});
    void editVideo(Json video = Json{});
    void editButton(Json button = Json{});
    void startWizard(bool playlist = false);
    void showLiveDetails(Json history);
    void liveCommand(long id, const std::string& action, Json parameters = Json{});
    void replay(Json history, bool beginning = false, int index = -1);
    void linkProfile(Json history);
    void showMixer(long id);
    void showDeck(long id);
    void showSnapshot(const Json& object);
    void playFile(const QString& path);
    void playSpot(long id);
    void refreshLiveRow(int row, const Json& record);
    void exportData();
    void importData();
    void removeRecord(const QString& kind, long id);
    bool confirm(const char* key, const QString& subject = {});
    void run(const std::function<void()>& action);
    void runAsync(std::function<Json()> job, std::function<void(Json)> finished = {});
    void showError(const std::exception& error);
    void addAction(const char* key, const std::function<void()>& action, QWidget* parent = nullptr,
                   const QString& icon = {});
    QTableWidget* makeTable(const QStringList& headers);
    void addPager(int size);
    void addDestinationCards(const QList<QStringList>& entries);
    void addDashboardCards(const QList<QStringList>& entries, const QString& section = {});
    void updatePreview();

  protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  public:
    NativeWindow(Studio& studio, fs::path resources);
    ~NativeWindow() override;
    void navigate(const QString& key);
    void sample();
    void setLanguage(const QString& language);
    void setTheme(const QString& theme);

    QString language() const {
        return translations.language();
    }

    QStringList languages() const {
        return translations.languages();
    }

    void editScene(Json scene, std::function<void(Json)> saved = {});
};

int runNativeApp(int argc, char** argv, Studio& studio, const fs::path& resources);
bool activateNativeApp(char** argv, const fs::path& root);
} // namespace orbis

#include "history_cleanup_service.hpp"
#include "native_controls.hpp"
#include "native_theme.hpp"
#include "native_window.hpp"
#include "platform_preview.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSlider>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

namespace orbis {
namespace {
QString text(const Json& value) {
    return QString::fromStdString(value.str());
}

QLabel* caption(const QString& value, QWidget* parent) {
    auto label = new QLabel(value, parent);
    label->setObjectName("muted");
    label->setWordWrap(true);
    label->setTextFormat(Qt::PlainText);
    return label;
}

void clearLayout(QLayout* layout) {
    while (auto item = layout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        } else if (item->layout())
            clearLayout(item->layout());
        delete item;
    }
}

const char* statusKey(const std::string& status) {
    return status == "LIVE"           ? "liveStatusLive"
           : status == "STARTING"     ? "liveStatusConnecting"
           : status == "RECONNECTING" ? "liveStatusReconnecting"
           : status == "ERROR"        ? "liveStatusError"
           : status == "ENDED"        ? "liveStatusEnded"
           : status == "STOPPED"      ? "liveStatusStopped"
                                      : "liveStatusOffline";
}

} // namespace

void NativeWindow::buildLive(bool history) {
    if (history) {
        auto heading = body->findChild<QFrame*>("pageHeading");
        auto cleanup = new QWidget(heading);
        cleanup->setObjectName("historyCleanup");
        auto row = new QHBoxLayout(cleanup);
        row->setContentsMargins(0, 0, 0, 0);
        Json config;
        auto channels = studio.store.list("channels");
        for (size_t i = 0; i < channels.size(); ++i)
            if (channels.at(i).get("isActive").flag()) {
                config = channels.at(i);
                break;
            }
        auto enabled = new ToggleSwitch(cleanup);
        auto enabledColumn = new QVBoxLayout;
        enabledColumn->addWidget(caption(t("autoCleanupEnabled"), cleanup));
        enabledColumn->addWidget(enabled);
        enabled->setObjectName("autoCleanupEnabled");
        enabled->setChecked(config.get("autoCleanupEnabled").flag());
        row->addLayout(enabledColumn);
        auto picks = new QWidget(cleanup);
        auto choices = new QHBoxLayout(picks);
        choices->setContentsMargins(0, 0, 0, 0);
        auto interval = new QComboBox(picks);
        interval->setObjectName("autoCleanupIntervalMonths");
        for (int months : {1, 2, 3, 6, 12})
            interval->addItem(
                months == 1 ? t("intervalEveryMonth")
                            : t("intervalEveryMonths").replace("{0}", QString::number(months)),
                months);
        interval->setCurrentIndex(
            std::max(0, interval->findData(int(config.get("autoCleanupIntervalMonths").num(1)))));
        auto age = new QComboBox(cleanup);
        age->setObjectName("autoCleanupOlderThanMonths");
        const QStringList labels = {t("olderThanYesterday"), t("olderThan1Month"),
                                    t("olderThan3Months"),   t("olderThan6Months"),
                                    t("olderThan1Year"),     t("olderThan2Years"),
                                    t("olderThanNow")};
        const QList<int> values = {0, 1, 3, 6, 12, 24, -1};
        for (int i = 0; i < values.size(); ++i)
            age->addItem(labels[i], values[i]);
        age->setCurrentIndex(
            std::max(0, age->findData(int(config.get("autoCleanupOlderThanMonths").num()))));
        auto intervalColumn = new QVBoxLayout;
        intervalColumn->addWidget(caption(t("autoCleanupIntervalMonths"), picks));
        intervalColumn->addWidget(interval);
        auto ageColumn = new QVBoxLayout;
        ageColumn->addWidget(caption(t("autoCleanupOlderThanMonths"), picks));
        ageColumn->addWidget(age);
        choices->addLayout(intervalColumn);
        choices->addLayout(ageColumn);
        addAction(
            "save",
            [this, config, enabled, interval, age] {
                if (!config.has("id")) {
                    statusBar()->showMessage(t("wizardNoActiveConfigurationsMessage"));
                    return;
                }
                auto value = studio.store.get("channels", config.get("id").num());
                value.set("autoCleanupEnabled", enabled->isChecked());
                value.set("autoCleanupIntervalMonths", double(interval->currentData().toInt()));
                value.set("autoCleanupOlderThanMonths", double(age->currentData().toInt()));
                studio.store.save("channels", value, value.get("id").num());
            },
            picks, "save");
        row->addWidget(picks);
        // The age is also used for a manual cleanup while the scheduler is off.
        age->setVisible(true);
        picks->setVisible(enabled->isChecked());
        connect(enabled, &QCheckBox::toggled, picks, &QWidget::setVisible);
        addAction(
            "runCleanupNow",
            [this, age] {
                cleanupDismissed.clear();
                studio.historyCleanup->tryStart(age->currentData().toInt());
            },
            cleanup, "restart");
        heading->layout()->addWidget(cleanup);
        auto progress = new QFrame(this);
        progress->setObjectName("cleanupProgress");
        auto column = new QVBoxLayout(progress);
        cleanupLabel = caption({}, progress);
        cleanupBar = new QProgressBar(progress);
        column->addWidget(cleanupLabel);
        column->addWidget(cleanupBar);
        cleanupBar->setTextVisible(false);
        cleanupBar->setFixedHeight(6);
        auto close = new QToolButton(progress);
        close->setObjectName("closeCleanup");
        close->setIcon(studioIcon("close"));
        close->setToolTip(t("close"));
        column->insertWidget(0, close, 0, Qt::AlignRight);
        connect(close, &QToolButton::clicked, this, [this, progress] {
            cleanupDismissed = "dismissed";
            progress->hide();
        });
        progress->setFixedWidth(460);
        progress->hide();
    }
    auto filters = new QWidget(body);
    auto row = new QHBoxLayout(filters);
    row->setContentsMargins(0, 0, 0, 0);
    statusFilter = new QComboBox(filters);
    statusFilter->setObjectName("statusFilter");
    statusFilter->addItem("—", "");
    for (const auto& status :
         {"LIVE", "OFFLINE", "ENDED", "ERROR", "STOPPED", "STARTING", "RECONNECTING"})
        statusFilter->addItem(t(statusKey(status)), status);
    channelFilter = new QComboBox(filters);
    channelFilter->setObjectName("channelFilter");
    channelFilter->addItem("—", "");
    auto addField = [&](const char* key, QWidget* control) {
        auto column = new QVBoxLayout;
        column->addWidget(caption(t(key), filters));
        column->addWidget(control);
        control->setAccessibleName(t(key));
        if (!history || key != QString("searchByTitle"))
            control->setFixedWidth(162);
        row->addLayout(column, history && key == QString("searchByTitle") ? 1 : 0);
    };
    if (history) {
        addField("channel", channelFilter);
        addField("liveStatus", statusFilter);
    } else {
        addField("liveStatus", statusFilter);
        addField("channel", channelFilter);
    }
    if (!history) {
        platformFilter = new QComboBox(filters);
        platformFilter->setObjectName("platformFilter");
        platformFilter->addItem("—", "");
        for (const auto& name :
             {"Custom", "Twitch", "YouTube", "Kick", "Facebook", "TikTok", "Rumble", "X"})
            platformFilter->addItem(name, name);
        addField("platform", platformFilter);
        connect(platformFilter, &QComboBox::currentIndexChanged, this, [this] {
            currentPage = 0;
            refreshRows();
        });
    } else {
        search = new QLineEdit(filters);
        search->setObjectName("search");
        search->setClearButtonEnabled(true);
        search->setPlaceholderText(t("searchByTitle"));
        addField("searchByTitle", search);
        addAction(
            "search",
            [this] {
                currentPage = 0;
                refreshRows();
            },
            filters, "search");
        addAction(
            "clearFilters",
            [this] {
                channelFilter->setCurrentIndex(0);
                statusFilter->setCurrentIndex(0);
                search->clear();
                currentPage = 0;
                refreshRows();
            },
            filters, "close");
        auto delay = new QTimer(search);
        delay->setSingleShot(true);
        delay->setInterval(400);
        connect(search, &QLineEdit::textChanged, delay, qOverload<>(&QTimer::start));
        connect(search, &QLineEdit::returnPressed, this, [this] {
            currentPage = 0;
            refreshRows();
        });
        connect(delay, &QTimer::timeout, this, [this] {
            currentPage = 0;
            refreshRows();
        });
    }
    if (!history)
        row->addStretch();
    filters->setObjectName(history ? "historyFilters" : "liveFilters");
    bodyLayout->addWidget(filters);
    for (auto pick : {channelFilter, statusFilter})
        connect(pick, &QComboBox::currentIndexChanged, this, [this] {
            currentPage = 0;
            refreshRows();
        });
    table = history ? makeTable({t("title"), t("liveStatus"), t("channel"), t("platform"),
                                 t("path"), t("data"), t("outcome"), ""})
                    : makeTable({t("liveStatus"), t("title"), t("channel"),
                                 t("startDate").remove(':').trimmed(), ""});
    addPager(history ? 20 : 10);
}

void NativeWindow::refreshHistory() {
    if (!table || QApplication::activePopupWidget())
        return;
    auto rows = state.get("liveRows");
    auto choice = channelFilter->currentData();
    QStringList channels;
    for (size_t i = 0; i < rows.size(); ++i)
        if (!channels.contains(text(rows.at(i).get("channelName"))))
            channels.append(text(rows.at(i).get("channelName")));
    channels.sort();
    channelFilter->blockSignals(true);
    channelFilter->clear();
    channelFilter->addItem("—", "");
    for (const auto& channel : channels)
        if (!channel.isEmpty())
            channelFilter->addItem(channel, channel);
    channelFilter->setCurrentIndex(std::max(0, channelFilter->findData(choice)));
    channelFilter->blockSignals(false);
    std::vector<Json> selected;
    for (size_t i = 0; i < rows.size(); ++i) {
        auto row = rows.at(i);
        if (!channelFilter->currentData().toString().isEmpty() &&
            text(row.get("channelName")) != channelFilter->currentData().toString())
            continue;
        auto files = row.get("request").get("files");
        const auto count = std::max<size_t>(1, files.size());
        for (size_t n = 0; n < count; ++n) {
            auto item = row.clone();
            auto path = files.at(n).str(row.get("source").str());
            item.set("historyPath", path);
            item.set("videoIndex", double(n));
            const auto result = row.get("videoResults").at(n);
            if (result.has("status")) {
                item.set("status", result.get("status"));
                item.set("error", result.get("error"));
            }
            if (!statusFilter->currentData().toString().isEmpty() &&
                text(item.get("status")) != statusFilter->currentData().toString())
                continue;
            if (files.size())
                item.set("title", fs::path(path).filename().string());
            if (search->text().isEmpty() ||
                text(item.get("title")).contains(search->text(), Qt::CaseInsensitive))
                selected.push_back(item);
        }
    }
    const int pages = std::max(1, int((selected.size() + 19) / 20));
    currentPage = std::clamp(currentPage, 0, pages - 1);
    pageLabel->setText(t("positionOf")
                           .replace("{0}", QString::number(currentPage + 1))
                           .replace("{1}", QString::number(pages)));
    pageLabel->parentWidget()->findChild<QPushButton*>("previousPage")->setEnabled(currentPage > 0);
    pageLabel->parentWidget()
        ->findChild<QPushButton*>("nextPage")
        ->setEnabled(currentPage + 1 < pages);
    updatePager(pages);
    emptyState->setVisible(selected.empty());
    table->setVisible(!selected.empty());
    body->findChild<QPushButton*>("clearFilters")
        ->setVisible(!search->text().trimmed().isEmpty() ||
                     !channelFilter->currentData().toString().isEmpty() ||
                     !statusFilter->currentData().toString().isEmpty());
    table->setRowCount(0);
    for (size_t i = currentPage * 20; i < std::min(selected.size(), size_t((currentPage + 1) * 20));
         ++i) {
        const auto record = selected[i];
        const int row = table->rowCount();
        table->insertRow(row);
        const QStringList values = {
            text(record.get("title")),
            t(statusKey(record.get("status").str())),
            text(record.get("channelName")),
            text(record.get("platform")),
            text(record.get("historyPath")),
            QDateTime::fromString(text(record.get("startedAt")), Qt::ISODate)
                .toLocalTime()
                .toString("dd-MM-yyyy HH:mm:ss"),
            text(record.get("error"))};
        for (int c = 0; c < values.size(); ++c) {
            auto item = new QTableWidgetItem(values[c]);
            item->setData(Qt::UserRole, qlonglong(record.get("id").num()));
            item->setToolTip(values[c]);
            table->setItem(row, c, item);
        }
        auto statusCell = new QWidget(table);
        auto statusRow = new QHBoxLayout(statusCell);
        statusRow->setContentsMargins(8, 0, 8, 0);
        auto badge = new QLabel(values[1], statusCell);
        auto token = text(record.get("status")).toLower();
        badge->setProperty("badge",
                           token == "starting" || token == "reconnecting" ? "connecting" : token);
        statusRow->addWidget(badge);
        statusRow->addStretch();
        table->setCellWidget(row, 1, statusCell);
        auto platform = new QWidget(table);
        auto platformRow = new QHBoxLayout(platform);
        platformRow->setContentsMargins(8, 0, 8, 0);
        auto mark = new QLabel(platform);
        mark->setPixmap(platformMark(values[3]).pixmap(18, 18));
        platformRow->addWidget(mark);
        platformRow->addWidget(new QLabel(values[3], platform));
        platformRow->addStretch();
        if (record.get("platformKnown").str().empty()) {
            auto unknown = caption("–", platform);
            platformRow->addWidget(unknown);
            mark->hide();
            for (auto child : platform->findChildren<QLabel*>())
                if (child != unknown && child != mark)
                    child->hide();
        }
        table->setCellWidget(row, 3, platform);
        if (values[6].size() > 120) {
            auto full = new QPushButton(t("viewFullMessage"), table);
            full->setObjectName("viewFullMessage");
            connect(full, &QPushButton::clicked, this,
                    [this, message = values[6]] { showOutcome(message); });
            table->setCellWidget(row, 6, full);
        }
        auto actions = new QWidget(table);
        auto actionsRow = new QHBoxLayout(actions);
        actionsRow->setContentsMargins(4, 0, 4, 0);
        actionsRow->setSpacing(4);
        if (record.get("status").str() == "ERROR" && !values[6].isEmpty()) {
            auto report = new QToolButton(actions);
            report->setObjectName("reportError");
            report->setProperty("rowAction", true);
            report->setIcon(studioIcon("report"));
            report->setToolTip(t("reportError"));
            report->setAccessibleName(t("reportError"));
            actionsRow->addWidget(report);
            connect(report, &QToolButton::clicked, this,
                    [this, record] { showErrorReport(record, record.get("videoIndex").num()); });
        }
        auto remove = new QToolButton(table);
        remove->setObjectName("deleteLiveHistory");
        remove->setProperty("rowAction", true);
        remove->setIcon(studioIcon("delete", QColor("#ff453a")));
        remove->setToolTip(t("deleteLiveHistory"));
        remove->setAccessibleName(t("deleteLiveHistory"));
        remove->setEnabled(!record.get("running").flag());
        connect(remove, &QToolButton::clicked, this, [this, record] {
            run([&] {
                if (confirm("deleteLiveHistoryConfirm"))
                    studio.store.removeHistoryIfInactive(record.get("id").num(),
                                                         record.get("startedAt").str());
            });
        });
        actionsRow->addWidget(remove);
        actionsRow->addStretch();
        table->setCellWidget(row, 7, actions);
        table->setRowHeight(row, 48);
    }
    auto progress = studio.historyCleanup->progress();
    auto popup = cleanupLabel->parentWidget();
    popup->adjustSize();
    popup->move(std::max(12, (width() - popup->width()) / 2),
                std::max(12, height() - popup->height() - 24));
    popup->setVisible(cleanupDismissed.isEmpty() && progress.has("totalVideos"));
    popup->raise();
    cleanupLabel->setText(
        progress.get("error").str().empty()
            ? (progress.get("running").flag() ? t("cleanupRunning") : t("cleanupInProgress")) +
                  "\n" +
                  t("cleanupProgressCount")
                      .replace("{0}", QString::number(progress.get("deletedVideos").num()))
                      .replace("{1}", QString::number(progress.get("totalVideos").num())) +
                  "\n" + text(progress.get("currentVideo"))
            : t("cleanupError").replace("{0}", text(progress.get("error"))));
    cleanupBar->setVisible(progress.get("error").str().empty());
    cleanupBar->setMaximum(std::max(1, int(progress.get("totalVideos").num())));
    cleanupBar->setValue(progress.get("deletedVideos").num());
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->setColumnWidth(1, 105);
    table->setColumnWidth(2, 125);
    table->setColumnWidth(3, 130);
    table->setColumnWidth(4, 240);
    table->setColumnWidth(5, 170);
    table->setColumnWidth(6, 155);
    table->setColumnWidth(7, 80);
    body->findChild<QPushButton*>("runCleanupNow")->setEnabled(!progress.get("running").flag());
}

void NativeWindow::unlockVideos() {
    QDialog dialog(this);
    dialog.setObjectName("unlockDialog");
    dialog.setWindowTitle(t("unLockVideo"));
    auto column = new QVBoxLayout(&dialog);
    column->addWidget(caption(t("hereYouCanUnlockVideos") + "\n" + t("appClosedUnexpectedly") +
                                  "\n" + t("videoLockedUnexpectedly") + "\n" +
                                  t("liveStreamStopped"),
                              &dialog));
    const auto rows = studio.store.list("videos");
    for (size_t i = 0; i < rows.size(); ++i)
        if (rows.at(i).get("isLocked").flag()) {
            auto row = new QWidget(&dialog);
            auto line = new QHBoxLayout(row);
            line->addWidget(caption(text(rows.at(i).get("videoPath")), row), 1);
            auto unlock = new QPushButton(t("unlockVideoButton"), row);
            unlock->setIcon(studioIcon("unlock"));
            line->addWidget(unlock);
            connect(unlock, &QPushButton::clicked, &dialog, [this, record = rows.at(i), row] {
                run([&] {
                    auto value = record.clone();
                    value.set("isLocked", false);
                    studio.store.save("videos", value, value.get("id").num());
                    row->hide();
                });
            });
            column->addWidget(row);
        }
    auto close = new QPushButton(t("back"), &dialog);
    column->addWidget(close);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.resize(720, 440);
    dialog.exec();
    rebuild();
}

void NativeWindow::buildPreview() {
    livePicker = new QComboBox(body);
    livePicker->setObjectName("livePicker");
    livePicker->setAccessibleName(t("runningLives"));
    auto picker = new QWidget(body);
    picker->setObjectName("previewPicker");
    auto pickRow = new QHBoxLayout(picker);
    pickRow->addWidget(caption(t("runningLives"), picker));
    pickRow->addWidget(livePicker, 1);
    bodyLayout->addWidget(picker);
    auto empty = new QFrame(body);
    empty->setObjectName("previewEmpty");
    auto emptyColumn = new QVBoxLayout(empty);
    emptyColumn->setContentsMargins(16, 40, 16, 40);
    auto message = caption(t("noLiveRunning"), empty);
    message->setObjectName("previewEmptyMessage");
    message->setAlignment(Qt::AlignCenter);
    emptyColumn->addWidget(message);
    addAction("startLiveButton", [this] { startWizard(); }, empty, "play");
    addAction("manageLiveButton", [this] { navigate("live"); }, empty, "screen");
    bodyLayout->addWidget(empty);
    auto heading = body->findChild<QFrame*>("pageHeading");
    auto actions = new QWidget(heading);
    actions->setObjectName("previewHeaderActions");
    auto headRow = new QHBoxLayout(actions);
    headRow->setContentsMargins(0, 0, 0, 0);
    addAction("playSpot", [this] { playSpot(watched); }, actions, "spot");
    addAction(
        "stopLiveIconLabel",
        [this] {
            if (confirm("stopLive"))
                runAsync([this, id = watched] {
                    if (auto session = studio.session(id))
                        session->stop();
                    return Json{};
                });
        },
        actions, "stop");
    addAction(
        "liveNow",
        [this] {
            auto session = studio.session(watched);
            if (!session || session->done())
                return;
            auto channel = studio.store.get("channels", session->snapshot().get("channelId").num());
            runAsync([channel] { return platformPlayer(channel); },
                     [](Json result) {
                         const auto url = result.get("publicUrl").str();
                         if (!url.empty())
                             QDesktopServices::openUrl(QUrl(QString::fromStdString(url)));
                     });
        },
        actions, "screen");
    actions->findChild<QPushButton*>("liveNow")->setProperty("liveIndicator", true);
    heading->layout()->addWidget(actions);
    auto indicator = new QLabel(t("liveNow"), actions);
    indicator->setObjectName("previewLiveIndicator");
    indicator->setFixedHeight(36);
    indicator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    indicator->setProperty("badge", "live");
    headRow->addWidget(indicator);
    previewContent = new QWidget(body);
    previewContent->setObjectName("previewContent");
    auto columns = new QHBoxLayout(previewContent);
    columns->setContentsMargins(0, 0, 0, 0);
    columns->setSpacing(12);
    auto main = new QWidget(previewContent);
    auto mainColumn = new QVBoxLayout(main);
    mainColumn->setContentsMargins(0, 0, 0, 0);
    auto stage = new QFrame(main);
    stage->setObjectName("previewStage");
    auto stageColumn = new QVBoxLayout(stage);
    stageColumn->setContentsMargins(10, 10, 10, 10);
    stageColumn->setSpacing(10);
    mainColumn->addWidget(stage);
    preview = new QLabel(t("previewWaiting"), stage);
    preview->setObjectName("livePreview");
    preview->setMinimumSize(300, 220);
    preview->setAlignment(Qt::AlignCenter);
    stageColumn->addWidget(preview);
    previewFacts = caption({}, main);
    previewFacts->setObjectName("previewFacts");
    auto metadata = new QWidget(stage);
    auto metaRow = new QHBoxLayout(metadata);
    metaRow->setContentsMargins(0, 0, 0, 0);
    metaRow->addWidget(previewFacts, 1);
    auto time = caption({}, metadata);
    time->setObjectName("previewTime");
    metaRow->addWidget(time);
    stageColumn->addWidget(metadata);
    auto badge = new QLabel(preview);
    badge->setObjectName("previewBadge");
    badge->move(10, 10);
    badge->setStyleSheet("background: #a6000000; color: white; padding: 4px 9px; border-radius: "
                         "6px; font-size: 11px;");
    previewDeck = new QFrame(main);
    previewDeck->setObjectName("card");
    new QVBoxLayout(previewDeck);
    mainColumn->addWidget(previewDeck);
    previewMixer = new QFrame(main);
    previewMixer->setObjectName("card");
    new QVBoxLayout(previewMixer);
    mainColumn->addWidget(previewMixer);
    mainColumn->addStretch();
    columns->addWidget(main, 155);
    auto params = new QFrame(previewContent);
    params->setObjectName("previewParameters");
    params->setMinimumWidth(320);
    auto form = new QFormLayout(params);
    form->setContentsMargins(18, 18, 18, 18);
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    form->addRow(caption(t("liveOutput"), params));
    auto facts = caption({}, params);
    facts->setObjectName("outputFacts");
    facts->hide();
    auto factPanel = new QWidget(params);
    auto factGrid = new QGridLayout(factPanel);
    factGrid->setContentsMargins(0, 0, 0, 0);
    factGrid->setHorizontalSpacing(14);
    factGrid->setVerticalSpacing(10);
    const QList<QPair<QString, QString>> factNames = {
        {"outputSize", t("resolution")}, {"outputFps", t("frameRate")},
        {"outputCodec", t("codec")},     {"sourceSize", t("sourceVideo")},
        {"sourceFps", t("frameRate")},   {"sourceAudio", t("audioChannels")}};
    for (int i = 0; i < factNames.size(); ++i) {
        auto cell = new QWidget(factPanel);
        auto col = new QVBoxLayout(cell);
        col->setContentsMargins(0, 0, 0, 0);
        col->setSpacing(1);
        col->addWidget(caption(factNames[i].second, cell));
        auto field = new QLabel("–", cell);
        field->setObjectName(factNames[i].first);
        field->setStyleSheet("font-weight: 600;");
        col->addWidget(field);
        factGrid->addWidget(cell, i / 3, i % 3);
    }
    form->addRow(factPanel);
    previewParameterTimer = new QTimer(params);
    previewParameterTimer->setSingleShot(true);
    previewParameterTimer->setInterval(500);
    connect(previewParameterTimer, &QTimer::timeout, this, &NativeWindow::applyPreviewParameters);
    auto number = [&](const char* field, const char* label, double low, double high,
                      int decimals = 0) {
        auto input = new QDoubleSpinBox(params);
        input->setObjectName(QString("param_") + field);
        input->setRange(low, high);
        input->setDecimals(decimals);
        form->addRow(t(label), input);
        connect(input, &QDoubleSpinBox::valueChanged, previewParameterTimer,
                qOverload<>(&QTimer::start));
        return input;
    };
    auto choice = [&](const char* field, const char* label, const QStringList& values) {
        auto input = new QComboBox(params);
        input->setObjectName(QString("param_") + field);
        input->addItems(values);
        form->addRow(t(label), input);
        connect(input, &QComboBox::currentIndexChanged, previewParameterTimer,
                qOverload<>(&QTimer::start));
        return input;
    };
    form->addRow(caption(t("videoParameters"), params));
    const auto catalog = Json::parse(readFile(resources / "codec-catalog.json"));
    auto catalogChoice = [&](const char* field, const char* label, const char* group,
                             const QStringList& supported, bool numeric) {
        auto input = new QComboBox(params);
        input->setObjectName(QString("param_") + field);
        const auto entries = catalog.get(group);
        for (size_t i = 0; i < entries.size(); ++i) {
            auto entry = entries.at(i);
            input->addItem(text(entry.get("label")), numeric ? QVariant(int(entry.get("id").num()))
                                                             : QVariant(text(entry.get("name"))));
            if (!supported.contains(text(entry.get("name"))))
                qobject_cast<QStandardItemModel*>(input->model())
                    ->item(input->count() - 1)
                    ->setEnabled(false);
        }
        form->addRow(t(label), input);
        connect(input, &QComboBox::currentIndexChanged, previewParameterTimer,
                qOverload<>(&QTimer::start));
        return input;
    };
    catalogChoice("videoCodec", "codec", "VideoCodecs", {"libx264"}, true);
    catalogChoice("videoCodecName", "videoCodecName", "VideoCodecNames",
                  {"libx264", "h264_nvenc", "h264_qsv"}, false);
    catalogChoice("pixelFormat", "pixelFormat", "PixelFormats", {"yuv420p"}, false);
    auto videoRate = number("videoBitrate", "bitrate", .1, 100, 3);
    videoRate->setProperty("jsonScale", 1000.);
    videoRate->setSuffix(" Mbit/s");
    number("gopSize", "gopSize", 0.1, 10, 1)->setEnabled(false);
    form->addRow(caption(t("gopNotEditable"), params));
    auto keep = new QCheckBox(t("keepSource"), params);
    keep->setObjectName("param_keepSource");
    form->addRow(t("sourceVideo"), keep);
    number("videoWidth", "width", 16, 7680)->setSingleStep(2);
    number("videoHeight", "height", 16, 7680)->setSingleStep(2);
    number("frameRate", "frameRate", 1, 240, 3);
    connect(keep, &QCheckBox::toggled, this, [this, params](bool on) {
        for (auto key : {"param_videoWidth", "param_videoHeight"})
            params->findChild<QDoubleSpinBox*>(key)->parentWidget()->setVisible(!on);
        previewParameterTimer->start();
    });
    form->addRow(caption(t("audio"), params));
    catalogChoice("audioCodec", "audioCodec", "AudioCodecs", {"aac"}, true);
    choice("audioChannels", "audioChannels", {"1", "2"});
    number("audioBitrate", "audioBitrate", 32, 512)->setSuffix(" kbps");
    form->addRow(caption(t("applyNote"), params));
    auto notice = caption({}, params);
    notice->setObjectName("parameterNotice");
    form->addRow(notice);
    auto arrange = [&](const QStringList& keys, int cols) {
        auto panel = new QWidget(params);
        auto grid = new QGridLayout(panel);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(12);
        int i = 0;
        for (const auto& key : keys) {
            auto control = params->findChild<QWidget*>("param_" + key);
            auto taken = form->takeRow(control);
            auto cell = new QWidget(panel);
            auto col = new QVBoxLayout(cell);
            col->setContentsMargins(0, 0, 0, 0);
            col->setSpacing(5);
            if (taken.labelItem) {
                col->addWidget(taken.labelItem->widget());
                delete taken.labelItem;
            }
            delete taken.fieldItem;
            col->addWidget(control);
            grid->addWidget(cell, i / cols, i % cols);
            ++i;
        }
        return panel;
    };
    auto videoGrid =
        arrange({"videoCodec", "videoCodecName", "pixelFormat", "videoBitrate", "gopSize"}, 2);
    form->insertRow(3, videoGrid);
    auto dimensionGrid = arrange({"keepSource", "videoWidth", "videoHeight", "frameRate"}, 2);
    form->insertRow(5, dimensionGrid);
    for (const auto& key : {"param_audioCodec", "param_audioChannels"}) {
        auto control = params->findChild<QWidget*>(key);
        control->hide();
        if (auto label = form->labelForField(control))
            label->hide();
    }
    columns->addWidget(params, 100);
    auto platformNotice = new QFrame(body);
    platformNotice->setObjectName("platformNotice");
    auto noticeRow = new QHBoxLayout(platformNotice);
    auto noticeText = caption({}, platformNotice);
    noticeText->setObjectName("platformNoticeText");
    noticeRow->addWidget(noticeText, 1);
    auto dismiss = new QToolButton(platformNotice);
    dismiss->setIcon(studioIcon("close"));
    dismiss->setToolTip(t("close"));
    noticeRow->addWidget(dismiss);
    connect(dismiss, &QToolButton::clicked, this, [this, platformNotice] {
        platformNotice->setProperty("dismissedFor", qlonglong(watched));
        platformNotice->hide();
    });
    platformNotice->hide();
    bodyLayout->addWidget(platformNotice);
    auto facebook = new QLabel(body);
    facebook->setObjectName("facebookPreviewNotice");
    facebook->setWordWrap(true);
    facebook->setText(
        t("facebookGoLiveHelp") +
        " <a "
        "href='https://www.facebook.com/live/producer'>https://www.facebook.com/live/producer</a>");
    facebook->setOpenExternalLinks(true);
    facebook->hide();
    bodyLayout->addWidget(facebook);
    bodyLayout->addWidget(previewContent);
    connect(livePicker, &QComboBox::currentIndexChanged, this, [this] {
        watched = livePicker->currentData().toLongLong();
        previewControlsFor = 0;
        updatePreview();
    });
    updatePreview();
}

void NativeWindow::populatePreviewControls(const Json& snapshot) {
    previewProfile = snapshot.get("profile").clone();
    auto params = body->findChild<QFrame*>("previewParameters");
    for (auto spin : params->findChildren<QDoubleSpinBox*>()) {
        spin->blockSignals(true);
        const auto key = spin->objectName().mid(6).toStdString();
        spin->setValue(previewProfile.get(key).num(
            key == "videoWidth"     ? snapshot.get("scene").get("width").num(1280)
            : key == "videoHeight"  ? snapshot.get("scene").get("height").num(720)
            : key == "gopSize"      ? 2
            : key == "audioBitrate" ? 128
            : key == "videoBitrate" ? 4500
            : key == "frameRate"    ? 30
                                    : spin->minimum()));
        if (spin->property("jsonScale").isValid())
            spin->setValue(previewProfile.get(key).num(4500) /
                           spin->property("jsonScale").toDouble());
        spin->blockSignals(false);
    }
    for (auto pick : params->findChildren<QComboBox*>()) {
        pick->blockSignals(true);
        auto key = pick->objectName().mid(6).toStdString();
        auto value = previewProfile.get(key);
        QString selected = key == "audioChannels" ? QString::number(value.num(2)) : text(value);
        int index = pick->findText(selected);
        if (key == "videoCodec")
            index = pick->findData(int(value.num(27)));
        else if (key == "audioCodec")
            index = pick->findData(int(value.num(86018)));
        else if (key == "videoCodecName")
            index = pick->findData(QString::fromStdString(value.str("libx264")));
        else if (key == "pixelFormat")
            index = pick->findData(QString::fromStdString(value.str("yuv420p")));
        pick->setCurrentIndex(std::max(0, index));
        pick->blockSignals(false);
    }
    auto keep = params->findChild<QCheckBox*>("param_keepSource");
    keep->blockSignals(true);
    keep->setChecked(!previewProfile.has("videoWidth") ||
                     previewProfile.get("videoWidth").num() <= 0);
    keep->blockSignals(false);
    for (auto key : {"param_videoWidth", "param_videoHeight"})
        params->findChild<QDoubleSpinBox*>(key)->parentWidget()->setVisible(!keep->isChecked());
    previewParameterTimer->stop();
    clearLayout(previewMixer->layout());
    clearLayout(previewDeck->layout());
    auto mix = qobject_cast<QVBoxLayout*>(previewMixer->layout());
    mix->addWidget(caption(t("sourceVolumes"), previewMixer));
    auto items = snapshot.get("scene").get("items");
    for (size_t i = 0; i < items.size(); ++i) {
        auto item = items.at(i);
        if (!item.get("audioEnabled").flag(true) && item.get("sourceKind").str() == "OVERLAY")
            continue;
        auto row = new QWidget(previewMixer);
        auto line = new QHBoxLayout(row);
        auto mute = new QToolButton(row);
        mute->setCheckable(true);
        mute->setChecked(!item.get("audioEnabled").flag(true));
        mute->setIcon(studioIcon("volume"));
        mute->setToolTip(t("volumeMute"));
        mute->setObjectName(QString("sourceMute_%1").arg(i));
        line->addWidget(mute);
        auto main = new QVBoxLayout;
        main->setSpacing(4);
        main->addWidget(caption(QString::fromStdString(item.get("label").str(
                                    fs::path(item.get("sourceTarget").str()).filename().string())),
                                row));
        line->addLayout(main, 1);
        auto slider = new VolumeSlider(row);
        slider->setObjectName(QString("sourceVolume_%1").arg(i));
        slider->setRange(0, 200);
        slider->setSingleStep(5);
        slider->setValue(item.get("volume").num(100));
        main->addWidget(slider);
        auto reset = new QPushButton(QString::number(slider->value()) + "%", row);
        reset->setToolTip(t("volumeReset"));
        line->addWidget(reset);
        const long id = watched;
        auto apply = [this, id, i, slider, mute] {
            if (auto session = studio.session(id); !session || session->done())
                return;
            run([&] {
                Json command;
                command.set("index", double(i));
                command.set("volume", double(slider->value()));
                command.set("audioEnabled", !mute->isChecked());
                liveCommand(id, "volume", command);
            });
        };
        auto debounce = new QTimer(row);
        debounce->setSingleShot(true);
        debounce->setInterval(150);
        connect(debounce, &QTimer::timeout, row, apply);
        auto paint = [&, slider, mute, reset](int volume) {
            reset->setText(QString::number(volume) + "%");
            const QString color = volume == 0     ? "#98989d"
                                  : volume <= 100 ? "#30d158"
                                  : volume <= 150 ? "#ffd60a"
                                                  : "#ff453a";
            reset->setStyleSheet("color: " + color + "; border-radius: 12px; padding: 4px 10px;");
            mute->setIcon(studioIcon(volume == 0 ? "mute" : "volume", QColor(color)));
            mute->setToolTip(t(volume == 0 ? "volumeUnmute" : "volumeMute"));
            mute->setAccessibleName(mute->toolTip());
            QSignalBlocker block(mute);
            mute->setChecked(volume == 0);
            slider->setProperty("muted", volume == 0);
            slider->update();
        };
        connect(slider, &QSlider::valueChanged, row, [paint, debounce](int amount) {
            paint(amount);
            debounce->start();
        });
        connect(slider, &QSlider::sliderReleased, row, [debounce, apply] {
            debounce->stop();
            apply();
        });
        connect(mute, &QToolButton::clicked, row, [slider](bool on) {
            if (on) {
                slider->setProperty("beforeMute", slider->value());
                slider->setValue(0);
            } else
                slider->setValue(slider->property("beforeMute").isValid()
                                     ? slider->property("beforeMute").toInt()
                                     : 100);
        });
        connect(reset, &QPushButton::clicked, row, [slider] { slider->setValue(100); });
        paint(slider->value());
        mix->addWidget(row);
    }
    mix->addWidget(caption(t("sourceVolumesHint"), previewMixer));
    auto deck = qobject_cast<QVBoxLayout*>(previewDeck->layout());
    auto stripHeader = new QWidget(previewDeck);
    auto stripRow = new QHBoxLayout(stripHeader);
    stripRow->setContentsMargins(0, 0, 0, 0);
    auto title = caption(t("sceneStrip"), stripHeader);
    title->setObjectName("cardTitle");
    stripRow->addWidget(title, 1);
    deck->addWidget(stripHeader);
    auto controls = new QWidget(previewDeck);
    new QHBoxLayout(controls);
    addAction(
        "sceneConfigure",
        [this] {
            showDeck(watched);
            previewControlsFor = 0;
            updatePreview();
        },
        controls, "deck");
    addAction("sceneResume", [this] { liveCommand(watched, "resume"); }, controls, "play");
    stripRow->addWidget(controls);
    auto tilePanel = new QWidget(previewDeck);
    auto tileGrid = new QGridLayout(tilePanel);
    tileGrid->setContentsMargins(0, 0, 0, 0);
    tileGrid->setSpacing(10);
    deck->addWidget(tilePanel);
    const auto buttons = studio.store.list("buttons");
    for (size_t i = 0; i < buttons.size(); ++i) {
        auto record = buttons.at(i);
        auto button = new SceneTile(
            text(record.get("label")),
            QString::fromStdString(
                (studio.store.root / "scene-media" / record.get("mediaName").str()).string()),
            text(record.get("hotkey")),
            t(record.get("displayMode").str() == "in_scene" ? "sceneModeInScene"
                                                            : "sceneModeFullscreen"),
            tilePanel);
        button->setProperty("buttonId", qlonglong(record.get("id").num()));
        button->setEnabled(armedDeck == watched);
        button->setToolTip(t("sceneLocked"));
        connect(button, &QPushButton::clicked, this, [this, record] {
            run([&] {
                if (confirm(record.get("displayMode").str() == "in_scene" ? "sceneConfirmInScene"
                                                                          : "sceneConfirm",
                            text(record.get("label")))) {
                    Json data;
                    data.set("id", record.get("id"));
                    liveCommand(watched, "button", data);
                }
            });
        });
        tileGrid->addWidget(button, int(i) / 3, int(i) % 3);
    }
    if (armedDeck != watched)
        deck->addWidget(caption(t("sceneLocked"), previewDeck));
    if (buttons.size() == 0)
        deck->addWidget(caption(t("sceneEmpty"), previewDeck));
    previewControlsFor = watched;
}

void NativeWindow::applyPreviewParameters() {
    if (!watched || !studio.session(watched) || studio.session(watched)->done())
        return;
    auto value = previewProfile.clone();
    auto params = body->findChild<QFrame*>("previewParameters");
    if (!params)
        return;
    for (auto spin : params->findChildren<QDoubleSpinBox*>())
        value.set(spin->objectName().mid(6).toStdString(),
                  spin->value() * (spin->property("jsonScale").isValid()
                                       ? spin->property("jsonScale").toDouble()
                                       : 1));
    for (auto pick : params->findChildren<QComboBox*>()) {
        auto key = pick->objectName().mid(6).toStdString();
        if (key == "audioChannels")
            value.set(key, double(pick->currentText().toInt()));
        else if (key == "videoCodec" || key == "audioCodec")
            value.set(key, double(pick->currentData().toInt()));
        else if (pick->currentData().isValid())
            value.set(key, pick->currentData().toString().toStdString());
        else
            value.set(key, pick->currentText().toStdString());
    }
    if (params->findChild<QCheckBox*>("param_keepSource")->isChecked()) {
        value.set("videoWidth", Json(nullptr));
        value.set("videoHeight", Json(nullptr));
    }
    try {
        liveCommand(watched, "parameters", value);
        previewProfile = value;
        params->findChild<QLabel*>("parameterNotice")->setText(t("parametersApplied"));
        statusBar()->showMessage(t("parametersApplied"), 3000);
    } catch (const std::exception&) {
        params->findChild<QLabel*>("parameterNotice")
            ->setText(t("parametersFailed") + " · " + t("invalidParameter"));
    }
}

void NativeWindow::updatePreview() {
    if (!preview || !livePicker || QApplication::activePopupWidget())
        return;
    if (previewContent) {
        const bool stacked = previewContent->width() < 900;
        qobject_cast<QBoxLayout*>(previewContent->layout())
            ->setDirection(stacked ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
        auto params = body->findChild<QFrame*>("previewParameters");
        params->setMinimumWidth(stacked ? 0 : 320);
        params->setMaximumWidth(QWIDGETSIZE_MAX);
    }
    const auto live = state.get("live");
    livePicker->blockSignals(true);
    livePicker->clear();
    Json snapshot;
    for (size_t i = 0; i < live.size(); ++i) {
        const auto item = live.at(i);
        const auto status = item.get("status").str();
        if (status == "LIVE" || status == "STARTING" || status == "RECONNECTING")
            livePicker->addItem(text(item.get("channelName")) + " · " +
                                    text(item.get("scene").get("name")),
                                qlonglong(item.get("id").num()));
    }
    livePicker->setCurrentIndex(std::max(0, livePicker->findData(qlonglong(watched))));
    watched = livePicker->currentData().toLongLong();
    livePicker->blockSignals(false);
    body->findChild<QWidget*>("previewPicker")->setVisible(livePicker->count() > 1);
    body->findChild<QFrame*>("previewEmpty")->setVisible(!watched);
    previewContent->setVisible(watched != 0);
    auto header = body->findChild<QWidget*>("previewHeaderActions");
    header->setVisible(watched != 0);
    header->setEnabled(watched != 0);
    if (!watched) {
        body->findChild<QLabel*>("previewEmptyMessage")->setText(t("noLiveRunning"));
        body->findChild<QPushButton*>("startLiveButton")->show();
        body->findChild<QPushButton*>("manageLiveButton")->hide();
        previewControlsFor = 0;
        if (auto player = preview->findChild<PlatformPreview*>())
            delete player;
        preview->setProperty("playerFor", 0);
        body->findChild<QFrame*>("platformNotice")->hide();
        body->findChild<QLabel*>("facebookPreviewNotice")->hide();
        previewParameterTimer->stop();
        return;
    }
    for (size_t i = 0; i < live.size(); ++i)
        if (live.at(i).get("id").num() == watched)
            snapshot = live.at(i);
    const bool onAir = snapshot.get("status").str() == "LIVE";
    body->findChild<QFrame*>("previewEmpty")->setVisible(!onAir);
    previewContent->setVisible(onAir);
    header->setVisible(onAir);
    if (!onAir) {
        body->findChild<QLabel*>("previewEmptyMessage")
            ->setText(t(statusKey(snapshot.get("status").str())) + " · " +
                      text(snapshot.get("channelName")));
        body->findChild<QPushButton*>("startLiveButton")->hide();
        body->findChild<QPushButton*>("manageLiveButton")->show();
        if (auto player = preview->findChild<PlatformPreview*>())
            delete player;
        preview->setProperty("playerFor", 0);
        body->findChild<QFrame*>("platformNotice")->hide();
        body->findChild<QLabel*>("facebookPreviewNotice")->hide();
        previewParameterTimer->stop();
        return;
    }
    const bool publicPage = snapshot.get("platform").str("Custom") != "Custom";
    if (!publicPage) {
        if (auto player = preview->findChild<PlatformPreview*>())
            delete player;
        preview->setProperty("playerFor", 0);
        body->findChild<QFrame*>("platformNotice")->hide();
    }
    body->findChild<QLabel*>("facebookPreviewNotice")
        ->setVisible(snapshot.get("platform").str() == "Facebook");
    if (publicPage && preview->property("playerFor").toLongLong() != watched) {
        if (auto old = preview->findChild<PlatformPreview*>())
            delete old;
        preview->setProperty("playerFor", qlonglong(watched));
        auto player = new PlatformPreview(preview);
        player->setGeometry(preview->rect());
        connect(player, &PlatformPreview::playerState, this, [this, player](const QString& state) {
            auto notice = body->findChild<QFrame*>("platformNotice");
            if (!notice)
                return;
            auto message = notice->findChild<QLabel*>("platformNoticeText");
            const bool youtube = player->property("platform").toString() == "YouTube";
            const bool unverified = state == "UNVERIFIED" && youtube;
            const bool failed = state == "FAILED";
            message->setText(unverified ? t("youtubeUnverified") + " " + t("youtubeUnverifiedHelp")
                             : failed   ? t("embedFailed")
                                        : "");
            notice->setVisible((unverified || failed) &&
                               notice->property("dismissedFor").toLongLong() != watched);
        });
        auto channel = studio.store.get("channels", snapshot.get("channelId").num());
        player->setProperty("platform", text(snapshot.get("platform")));
        player->watch(channel);
    }
    header->findChild<QPushButton*>("liveNow")->setVisible(publicPage);
    header->findChild<QLabel*>("previewLiveIndicator")->setVisible(!publicPage);
    if (previewControlsFor != watched)
        populatePreviewControls(snapshot);
    const auto profile = snapshot.has("output") && snapshot.get("output").has("videoWidth")
                             ? snapshot.get("output")
                             : snapshot.get("profile");
    const bool portrait = profile.get("videoHeight").num(720) > profile.get("videoWidth").num(1280);
    const double aspect =
        profile.get("videoHeight").num(snapshot.get("scene").get("height").num(720)) /
        std::max(1.0, profile.get("videoWidth").num(snapshot.get("scene").get("width").num(1280)));
    preview->setFixedHeight(std::clamp(int(preview->width() * aspect), 220, portrait ? 520 : 600));
    if (auto player = preview->findChild<PlatformPreview*>())
        player->setGeometry(preview->rect());
    QPixmap frame(QString::fromStdString(
        (studio.store.root / "previews" / (std::to_string(watched) + ".jpg")).string()));
    if (!frame.isNull())
        preview->setPixmap(
            frame.scaled(preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        preview->setText(t(statusKey(snapshot.get("status").str())) + " · " + t("previewWaiting"));
    previewFacts->setText(text(snapshot.get("scene").get("name")) + "\n" +
                          text(snapshot.get("channelName")) +
                          (publicPage ? " · " + text(snapshot.get("platform")) : ""));
    auto formatTime = [](double seconds) {
        int total = std::max(0, int(seconds));
        return total >= 3600 ? QString("%1:%2:%3")
                                   .arg(total / 3600)
                                   .arg(total / 60 % 60, 2, 10, QChar('0'))
                                   .arg(total % 60, 2, 10, QChar('0'))
                             : QString("%1:%2").arg(total / 60).arg(total % 60, 2, 10, QChar('0'));
    };
    const auto source = snapshot.get("scene").get("sourceInfo");
    const double duration = source.get("format").get("duration").num();
    body->findChild<QLabel*>("previewTime")
        ->setText(formatTime(snapshot.get("offsetSeconds").num()) + " / " + formatTime(duration));
    auto badge = preview->findChild<QLabel*>("previewBadge");
    badge->setVisible(publicPage);
    badge->setText("● LIVE · " + text(snapshot.get("platform")));
    badge->adjustSize();
    badge->raise();
    auto fact = [&](const char* key, const QString& value) {
        body->findChild<QLabel*>(key)->setText(value);
    };
    fact("outputSize", QString::number(profile.get("videoWidth").num()) + "×" +
                           QString::number(profile.get("videoHeight").num()));
    fact("outputFps", QString::number(profile.get("frameRate").num()) + " fps");
    fact("outputCodec", QString::fromStdString(profile.get("videoCodecName").str("libx264")));
    QString sourceSize = "–", sourceFps = "–", sourceAudio = t("noAudioTrack");
    const auto streams = source.get("streams");
    for (size_t i = 0; i < streams.size(); ++i) {
        const auto stream = streams.at(i);
        if (stream.get("codec_type").str() == "video") {
            sourceSize = QString::number(stream.get("width").num()) + "×" +
                         QString::number(stream.get("height").num());
            const auto rate = text(stream.get("avg_frame_rate")).split('/');
            if (rate.size() == 2 && rate[1].toDouble() > 0)
                sourceFps =
                    QString::number(rate[0].toDouble() / rate[1].toDouble(), 'g', 5) + " fps";
        } else if (stream.get("codec_type").str() == "audio")
            sourceAudio = QString::number(stream.get("channels").num());
    }
    fact("sourceSize", sourceSize);
    fact("sourceFps", sourceFps);
    fact("sourceAudio", sourceAudio);
    if (auto resume = previewDeck->findChild<QPushButton*>("sceneResume"))
        resume->setVisible(snapshot.get("takeover").has("scene"));
}
} // namespace orbis

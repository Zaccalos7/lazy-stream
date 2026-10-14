#include "history_cleanup_service.hpp"
#include "native_theme.hpp"
#include "native_window.hpp"
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

QString clockTime(double seconds) {
    const int time = std::max(0, int(seconds));
    return QString("%1:%2:%3")
        .arg(time / 3600)
        .arg(time / 60 % 60, 2, 10, QChar('0'))
        .arg(time % 60, 2, 10, QChar('0'));
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
        auto enabled = new QCheckBox(t("autoCleanupEnabled"), cleanup);
        enabled->setObjectName("autoCleanupEnabled");
        enabled->setChecked(config.get("autoCleanupEnabled").flag());
        row->addWidget(enabled);
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
        choices->addWidget(interval);
        choices->addWidget(age);
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
        connect(enabled, &QCheckBox::toggled, this, [this, config](bool on) {
            if (!on && config.has("id"))
                run([&] {
                    auto value = studio.store.get("channels", config.get("id").num());
                    value.set("autoCleanupEnabled", false);
                    studio.store.save("channels", value, value.get("id").num());
                });
        });
        addAction(
            "runCleanupNow",
            [this, age] { studio.historyCleanup->tryStart(age->currentData().toInt()); }, cleanup,
            "restart");
        heading->layout()->addWidget(cleanup);
        auto progress = new QFrame(body);
        progress->setObjectName("cleanupProgress");
        auto column = new QVBoxLayout(progress);
        cleanupLabel = caption({}, progress);
        cleanupBar = new QProgressBar(progress);
        column->addWidget(cleanupLabel);
        column->addWidget(cleanupBar);
        bodyLayout->addWidget(progress);
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
        row->addLayout(column, 1);
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
        if (!statusFilter->currentData().toString().isEmpty() &&
            text(row.get("status")) != statusFilter->currentData().toString())
            continue;
        auto files = row.get("request").get("files");
        const auto count = std::max<size_t>(1, files.size());
        for (size_t n = 0; n < count; ++n) {
            auto item = row.clone();
            auto path = files.at(n).str(row.get("source").str());
            item.set("historyPath", path);
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
    pageLabel->parentWidget()->setVisible(!selected.empty());
    emptyState->setVisible(selected.empty());
    table->setVisible(!selected.empty());
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
                           token == "starting" || token == "reconnecting" ? "offline" : token);
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
        table->setCellWidget(row, 3, platform);
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
        table->setCellWidget(row, 7, remove);
        table->setRowHeight(row, 48);
    }
    auto progress = studio.historyCleanup->progress();
    cleanupLabel->parentWidget()->setVisible(progress.get("running").flag() ||
                                             !progress.get("error").str().empty());
    cleanupLabel->setText(
        progress.get("error").str().empty()
            ? t("cleanupProgressCount")
                      .replace("{0}", QString::number(progress.get("deletedVideos").num()))
                      .replace("{1}", QString::number(progress.get("totalVideos").num())) +
                  "\n" + text(progress.get("currentVideo"))
            : t("cleanupError").replace("{0}", text(progress.get("error"))));
    cleanupBar->setMaximum(std::max(1, int(progress.get("totalVideos").num())));
    cleanupBar->setValue(progress.get("deletedVideos").num());
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
    columns->setSpacing(16);
    auto main = new QWidget(previewContent);
    auto mainColumn = new QVBoxLayout(main);
    mainColumn->setContentsMargins(0, 0, 0, 0);
    preview = new QLabel(t("previewWaiting"), main);
    preview->setObjectName("livePreview");
    preview->setMinimumSize(300, 220);
    preview->setAlignment(Qt::AlignCenter);
    mainColumn->addWidget(preview);
    previewFacts = caption({}, main);
    previewFacts->setObjectName("previewFacts");
    mainColumn->addWidget(previewFacts);
    previewDeck = new QFrame(main);
    previewDeck->setObjectName("card");
    new QVBoxLayout(previewDeck);
    mainColumn->addWidget(previewDeck);
    previewMixer = new QFrame(main);
    previewMixer->setObjectName("card");
    new QVBoxLayout(previewMixer);
    mainColumn->addWidget(previewMixer);
    mainColumn->addStretch();
    columns->addWidget(main, 1);
    auto params = new QFrame(previewContent);
    params->setObjectName("previewParameters");
    params->setFixedWidth(300);
    auto form = new QFormLayout(params);
    form->setContentsMargins(18, 18, 18, 18);
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    form->addRow(caption(t("liveOutput"), params));
    auto facts = caption({}, params);
    facts->setObjectName("outputFacts");
    form->addRow(facts);
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
    number("videoBitrate", "bitrate", 100, 100000)->setSuffix(" kbps");
    number("gopSize", "gopSize", 0.1, 10, 1)->setEnabled(false);
    form->addRow(caption(t("gopNotEditable"), params));
    auto keep = new QCheckBox(t("keepSource"), params);
    keep->setObjectName("param_keepSource");
    form->addRow(t("sourceVideo"), keep);
    number("videoWidth", "width", 16, 7680)->setSingleStep(2);
    number("videoHeight", "height", 16, 7680)->setSingleStep(2);
    number("frameRate", "frameRate", 1, 120, 2);
    connect(keep, &QCheckBox::toggled, this, [this, params](bool on) {
        for (auto key : {"param_videoWidth", "param_videoHeight"})
            params->findChild<QDoubleSpinBox*>(key)->setEnabled(!on);
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
    columns->addWidget(params);
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
        params->findChild<QDoubleSpinBox*>(key)->setEnabled(!keep->isChecked());
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
        line->addWidget(caption(QString::fromStdString(item.get("label").str(
                                    fs::path(item.get("sourceTarget").str()).filename().string())),
                                row));
        auto slider = new QSlider(Qt::Horizontal, row);
        slider->setObjectName(QString("sourceVolume_%1").arg(i));
        slider->setRange(0, 200);
        slider->setSingleStep(5);
        slider->setValue(item.get("volume").num(100));
        line->addWidget(slider, 1);
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
        connect(slider, &QSlider::valueChanged, reset,
                [reset](int value) { reset->setText(QString::number(value) + "%"); });
        connect(slider, &QSlider::sliderReleased, row, apply);
        connect(mute, &QToolButton::toggled, row, apply);
        connect(reset, &QPushButton::clicked, row, [slider, mute, apply] {
            slider->setValue(100);
            mute->setChecked(false);
            apply();
        });
        mix->addWidget(row);
    }
    mix->addWidget(caption(t("sourceVolumesHint"), previewMixer));
    auto deck = qobject_cast<QVBoxLayout*>(previewDeck->layout());
    deck->addWidget(caption(t("sceneStrip"), previewDeck));
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
    deck->addWidget(controls);
    const auto buttons = studio.store.list("buttons");
    for (size_t i = 0; i < buttons.size(); ++i) {
        auto record = buttons.at(i);
        auto button = new QPushButton(text(record.get("label")), previewDeck);
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
        deck->addWidget(button);
    }
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
        value.set(spin->objectName().mid(6).toStdString(), spin->value());
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
        value.set("videoWidth", Json{});
        value.set("videoHeight", Json{});
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
        params->setMinimumWidth(300);
        params->setMaximumWidth(stacked ? QWIDGETSIZE_MAX : 300);
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
        previewParameterTimer->stop();
        return;
    }
    const bool publicPage = snapshot.get("platform").str("Custom") != "Custom";
    header->findChild<QPushButton*>("liveNow")->setVisible(publicPage);
    header->findChild<QLabel*>("previewLiveIndicator")->setVisible(!publicPage);
    if (previewControlsFor != watched)
        populatePreviewControls(snapshot);
    const auto profile = snapshot.get("profile");
    const bool portrait = profile.get("videoHeight").num(720) > profile.get("videoWidth").num(1280);
    const double aspect =
        profile.get("videoHeight").num(snapshot.get("scene").get("height").num(720)) /
        std::max(1.0, profile.get("videoWidth").num(snapshot.get("scene").get("width").num(1280)));
    preview->setFixedHeight(std::clamp(int(preview->width() * aspect), 220, portrait ? 520 : 600));
    QPixmap frame(QString::fromStdString(
        (studio.store.root / "previews" / (std::to_string(watched) + ".jpg")).string()));
    if (!frame.isNull())
        preview->setPixmap(
            frame.scaled(preview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        preview->setText(t(statusKey(snapshot.get("status").str())) + " · " + t("previewWaiting"));
    previewFacts->setText(text(snapshot.get("scene").get("name")) + " · " +
                          text(snapshot.get("channelName")) + "\n" +
                          clockTime(snapshot.get("offsetSeconds").num()));
    body->findChild<QLabel*>("outputFacts")
        ->setText(QString("%1 × %2 · %3 fps\n%4\n%5: %6")
                      .arg(profile.get("videoWidth").num())
                      .arg(profile.get("videoHeight").num())
                      .arg(profile.get("frameRate").num())
                      .arg(QString::fromStdString(profile.get("videoCodecName").str("libx264")))
                      .arg(t("audioChannels"))
                      .arg(profile.get("audioChannels").num(2)));
    if (auto resume = previewDeck->findChild<QPushButton*>("sceneResume"))
        resume->setVisible(snapshot.get("takeover").has("scene"));
}
} // namespace orbis

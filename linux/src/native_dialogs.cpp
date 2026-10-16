#include "media_job.hpp"
#include "native_controls.hpp"
#include "native_forms.hpp"
#include "native_theme.hpp"
#include "native_window.hpp"
#include "scene_service.hpp"
#include <QApplication>
#include <QButtonGroup>
#include <QDesktopServices>
#include <QEventLoop>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QShortcut>
#include <QSlider>
#include <QStackedWidget>
#include <QStandardItemModel>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTimer>
#include <QToolButton>
#include <QUuid>
#include <QWizard>
#include <QWizardPage>
#include <algorithm>

namespace orbis {
namespace {
class ChoiceDelegate : public QStyledItemDelegate {
  public:
    explicit ChoiceDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        QStyleOptionViewItem background(option);
        initStyleOption(&background, index);
        background.text.clear();
        background.icon = {};
        const auto style = background.widget ? background.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &background, painter, background.widget);
    }
};

class HotkeyCapture : public QPushButton {
  public:
    std::function<void(QString)> captured;

    explicit HotkeyCapture(const QString& text, QWidget* parent) : QPushButton(text, parent) {}

  protected:
    void keyPressEvent(QKeyEvent* event) override {
        if (!property("capturing").toBool()) {
            QPushButton::keyPressEvent(event);
            return;
        }
        const int code = event->key();
        if (code == Qt::Key_Escape) {
            setProperty("capturing", false);
            return;
        }
        QString key;
        if (code >= Qt::Key_A && code <= Qt::Key_Z)
            key = "Key" + QString(QChar(code));
        else if (code >= Qt::Key_0 && code <= Qt::Key_9)
            key = (event->modifiers().testFlag(Qt::KeypadModifier) ? "Numpad" : "Digit") +
                  QString(QChar(code));
        else if (code >= Qt::Key_F1 && code <= Qt::Key_F24)
            key = "F" + QString::number(code - Qt::Key_F1 + 1);
        else {
            const QMap<int, QString> keys = {{Qt::Key_Left, "ArrowLeft"},
                                             {Qt::Key_Right, "ArrowRight"},
                                             {Qt::Key_Up, "ArrowUp"},
                                             {Qt::Key_Down, "ArrowDown"},
                                             {Qt::Key_Insert, "Insert"},
                                             {Qt::Key_Delete, "Delete"},
                                             {Qt::Key_Home, "Home"},
                                             {Qt::Key_End, "End"},
                                             {Qt::Key_PageUp, "PageUp"},
                                             {Qt::Key_PageDown, "PageDown"},
                                             {Qt::Key_QuoteLeft, "Backquote"},
                                             {Qt::Key_Minus, "Minus"},
                                             {Qt::Key_Equal, "Equal"},
                                             {Qt::Key_BracketLeft, "BracketLeft"},
                                             {Qt::Key_BracketRight, "BracketRight"},
                                             {Qt::Key_Backslash, "Backslash"},
                                             {Qt::Key_Semicolon, "Semicolon"},
                                             {Qt::Key_Apostrophe, "Quote"},
                                             {Qt::Key_Comma, "Comma"},
                                             {Qt::Key_Period, "Period"},
                                             {Qt::Key_Slash, "Slash"}};
            key = keys.value(code);
        }
        if (key.isEmpty())
            return;
        QString prefix;
        if (event->modifiers().testFlag(Qt::ControlModifier))
            prefix += "Ctrl+";
        if (event->modifiers().testFlag(Qt::AltModifier))
            prefix += "Alt+";
        if (event->modifiers().testFlag(Qt::ShiftModifier))
            prefix += "Shift+";
        setProperty("capturing", false);
        if (captured)
            captured(prefix + key);
        event->accept();
    }
};

class ChoiceWizardPage : public QWizardPage {
  public:
    std::function<bool()> complete;

    bool isComplete() const override {
        return complete ? complete() : true;
    }

    void refresh() {
        emit completeChanged();
    }
};

// Cards fill the available width, up to five columns as in the Windows wizard.
class ChoiceList : public QListWidget {
  public:
    using QListWidget::QListWidget;

  protected:
    void resizeEvent(QResizeEvent* event) override {
        QListWidget::resizeEvent(event);
        const int available = viewport()->width() - 4;
        const int columns = std::clamp(available / 340, 1, 5);
        const int cell = available / columns;
        setSpacing(0);
        setGridSize(QSize(cell - 4, 86));
        for (int i = 0; i < count(); ++i)
            item(i)->setSizeHint(QSize(std::max(160, cell - 10), 76));
        doItemsLayout();
    }
};

void decorateChoices(QListWidget* list) {
    list->setItemDelegate(new ChoiceDelegate(list));
    list->setWrapping(true);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    for (int i = 0; i < list->count(); ++i) {
        auto item = list->item(i);
        auto row = new QWidget(list);
        row->setAttribute(Qt::WA_TransparentForMouseEvents);
        auto layout = new QHBoxLayout(row);
        layout->setContentsMargins(14, 8, 14, 8);
        layout->setSpacing(10);
        auto selected = new QRadioButton(row);
        selected->setAutoExclusive(false);
        selected->setChecked(list->currentRow() == i);
        layout->addWidget(selected);
        QObject::connect(list, &QListWidget::currentRowChanged, selected,
                         [selected, i](int index) { selected->setChecked(index == i); });
        auto icon = new QLabel(row);
        icon->setPixmap(item->icon().pixmap(20, 20));
        layout->addWidget(icon);
        auto column = new QVBoxLayout;
        const auto lines = item->text().split('\n');
        auto title = new QLabel(lines.first(), row);
        title->setWordWrap(true);
        title->setTextFormat(Qt::PlainText);
        title->setStyleSheet("font-weight: 600;");
        auto detail = new QLabel(lines.mid(1).join('\n').trimmed(), row);
        detail->setWordWrap(true);
        detail->setTextFormat(Qt::PlainText);
        detail->setObjectName("muted");
        detail->setStyleSheet("font-size: 11px;");
        column->setSpacing(2);
        column->addWidget(title);
        column->addWidget(detail);
        layout->addLayout(column, 1);
        item->setForeground(QBrush(Qt::transparent));
        item->setIcon({});
        list->setItemWidget(item, row);
    }
}

QString str(const Json& value) {
    return QString::fromStdString(value.str());
}

// Le scelte conservano l'ID numerico: nomi tradotti o duplicati non cambiano il record selezionato.
QList<QPair<QString, Json>> choices(const Json& rows, const char* label, bool activeOnly = false) {
    QList<QPair<QString, Json>> out;
    for (size_t i = 0; i < rows.size(); ++i) {
        auto row = rows.at(i);
        if (!activeOnly || row.get("isActive").flag())
            out.append({str(row.get(label)), row.get("id")});
    }
    return out;
}

// Copia privata senza usare la shell; il nome casuale evita collisioni fra upload omonimi.
std::string copyMedia(const fs::path& root, const QString& file) {
    fs::create_directories(root);
    auto name = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString() +
                fs::path(file.toStdString()).extension().string();
    fs::copy_file(file.toStdString(), root / name);
    fs::permissions(root / name, fs::perms::owner_read | fs::perms::owner_write);
    return name;
}
} // namespace

// Il form del canale mantiene ingest e chiave separati.
void NativeWindow::editChannel(Json channel) {
    NativeForm dialog(this, t(channel.has("id") ? "editModalTitle" : "addConfiguration"), channel);
    dialog.setObjectName("channelEditor");
    dialog.resize(std::max(800, std::min(1050, width() - 40)),
                  std::min(850, screen()->availableGeometry().height() - 40));
    dialog.buttons->button(QDialogButtonBox::Save)->setText(t("save"));
    dialog.buttons->button(QDialogButtonBox::Cancel)->setText(t("cancel"));
    const QMap<QString, QString> servers = {
        {"Twitch", "rtmp://live.twitch.tv/app"},
        {"YouTube", "rtmps://a.rtmps.youtube.com/live2"},
        {"Kick", "rtmps://fa723fc1b171.global-contribute.live-video.net/app"},
        {"Facebook", "rtmps://live-api-s.facebook.com:443/rtmp"},
        {"Custom", ""}};
    QList<QPair<QString, Json>> platforms;
    for (const auto& name : {"Twitch", "YouTube", "Kick", "Facebook", "Custom"})
        platforms.append({name, Json::string(name)});
    auto platform = dialog.choice("platform", t("platform"), platforms);
    platform->hide();
    dialog.form->labelForField(platform)->hide();
    auto choices = new QWidget(&dialog);
    auto column = new QVBoxLayout(choices);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(6);
    dialog.form->addRow(choices);
    for (const auto& entry : platforms) {
        auto card = new QFrame(choices);
        card->setObjectName("platformChoice");
        auto row = new QHBoxLayout(card);
        row->setContentsMargins(14, 8, 14, 8);
        auto pick = new QRadioButton(card);
        pick->setObjectName("platform_" + entry.first);
        pick->setChecked(platform->currentText() == entry.first);
        auto group = choices->findChild<QButtonGroup*>();
        if (!group)
            group = new QButtonGroup(choices);
        group->addButton(pick);
        row->addWidget(pick);
        auto mark = new QLabel(card);
        mark->setPixmap(platformMark(entry.first).pixmap(20, 20));
        row->addWidget(mark);
        row->addWidget(
            new QLabel(entry.first == "Facebook" ? "Facebook Gaming" : entry.first, card), 1);
        auto url = new QLabel(servers.value(entry.first), card);
        url->setObjectName("muted");
        row->addWidget(url);
        column->addWidget(card);
        connect(pick, &QRadioButton::clicked, &dialog, [platform, name = entry.first] {
            platform->setCurrentIndex(platform->findText(name));
        });
    }
    dialog.text("channelName", t("channelName"));
    auto ingest = dialog.text("streamUrl", t("url"));
    auto key = dialog.text("streamKey", t("key"), true);
    auto reveal = key->addAction(studioIcon("eye"), QLineEdit::TrailingPosition);
    reveal->setText(t("showStreamKey"));
    reveal->setCheckable(true);
    connect(reveal, &QAction::toggled, &dialog,
            [key](bool show) { key->setEchoMode(show ? QLineEdit::Normal : QLineEdit::Password); });
    dialog.grid(
        {{"channelName", t("channelName")}, {"streamUrl", t("url")}, {"streamKey", t("key")}}, 2);
    dialog.multiline("description", t("description"));
    auto active = dialog.check("isActive", t("active"), true);
    active->hide();
    auto sender = dialog.check("ffmpegSender", t("ffmpegSender"), true);
    sender->setToolTip(t("ffmpegSenderHelp"));
    auto senderHint = new QLabel(t("ffmpegSenderHelp"), &dialog);
    senderHint->setObjectName("muted");
    senderHint->setWordWrap(true);
    dialog.form->addRow(senderHint);
    auto facebook = new QLabel(t("facebookGoLiveHelp"), &dialog);
    facebook->setObjectName("muted");
    facebook->setWordWrap(true);
    dialog.form->addRow(facebook);
    auto ingestHint = new QLabel(t("ingestFromDashboard"), &dialog);
    ingestHint->setObjectName("ingestDashboardHint");
    ingestHint->setWordWrap(true);
    dialog.form->addRow(ingestHint);
    auto select = [&, platform, ingest, sender, senderHint, facebook, ingestHint](bool fill) {
        auto name = platform->currentText();
        if (fill && servers.contains(name))
            ingest->setText(servers.value(name));
        sender->setVisible(name == "YouTube");
        senderHint->setVisible(name == "YouTube");
        sender->setEnabled(name == "YouTube");
        if (fill && name == "YouTube") {
            sender->setChecked(true);
        }
        facebook->setVisible(name == "Facebook");
        ingestHint->setVisible(name == "Kick" || name == "Facebook");
    };
    connect(platform, &QComboBox::currentIndexChanged, &dialog, [&] { select(true); });
    select(!channel.has("id"));
    if (channel.has("id")) {
        auto remove = dialog.buttons->addButton(t("delete"), QDialogButtonBox::DestructiveRole);
        remove->setIcon(studioIcon("delete"));
        connect(remove, &QPushButton::clicked, &dialog, [&] {
            run([&] {
                if (confirm("deleteConfigurations")) {
                    studio.store.remove("channels", channel.get("id").num());
                    dialog.reject();
                }
            });
        });
    }
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto result = dialog.value();
            result.set("platformStreamName", platform->currentText().toLower().toStdString());
            validate("channels", result);
            studio.store.save("channels", result, channel.get("id").num());
            dialog.accept();
        });
    });
    dialog.exec();
    sample();
}

// Le opzioni sono quelle del motore FFmpeg, con limiti coerenti con la validazione del backend.
void NativeWindow::editProfile(Json profile, std::function<void(Json)> apply) {
    NativeForm dialog(this,
                      t(apply               ? "connectVideoSetting"
                        : profile.has("id") ? "editModalTitle"
                                            : "addNewAudioVideoConfiguration"),
                      profile);
    dialog.setObjectName("profileEditor");
    dialog.resize(std::max(900, std::min(1320, width() - 40)),
                  std::min(900, screen()->availableGeometry().height() - 40));
    dialog.buttons->button(QDialogButtonBox::Save)->setText(t("save"));
    dialog.buttons->button(QDialogButtonBox::Cancel)->setText(t("cancel"));
    auto active = dialog.check("isActive", t("active"), false);
    active->hide();
    auto presets = new QWidget(&dialog);
    auto presetRow = new QHBoxLayout(presets);
    presetRow->setContentsMargins(0, 0, 0, 0);
    auto presetHint = new QLabel(t("clickButtonToLoadVideoSettings"), presets);
    presetHint->setWordWrap(true);
    presetHint->setObjectName("muted");
    presetRow->addWidget(presetHint);
    const auto templates = studio.store.list("profiles");
    dialog.form->addRow(presets);
    dialog.text("title", t("videoSettingTitle"))->setPlaceholderText(t("titlePlaceholder"));
    dialog.section(t("video"));
    const auto catalog = Json::parse(readFile(resources / "codec-catalog.json"));
    auto codec = [&](const char* key, const char* label, const char* group,
                     const QStringList& supported, bool numeric) {
        QList<QPair<QString, Json>> values;
        const auto rows = catalog.get(group);
        for (size_t i = 0; i < rows.size(); ++i)
            values.append({str(rows.at(i).get("label")),
                           numeric ? rows.at(i).get("id") : rows.at(i).get("name")});
        auto input = dialog.choice(key, t(label), values);
        if (!profile.has(key)) {
            for (size_t i = 0; i < rows.size(); ++i)
                if (supported.contains(str(rows.at(i).get("name")))) {
                    input->setCurrentIndex(i);
                    break;
                }
        }
        return input;
    };
    codec("videoCodec", "codec", "VideoCodecs", {"libx264"}, true);
    codec("videoCodecName", "videoCodecName", "VideoCodecNames",
          {"libx264", "h264_nvenc", "h264_qsv", "h264_vaapi"}, false);
    codec("pixelFormat", "pixelFormat", "PixelFormats", {"yuv420p"}, false);
    QList<QPair<QString, Json>> formats;
    for (const auto& format : {"mp4", "mkv", "avi", "mov", "flv", "webm", "ts"})
        formats.append({QString(format).toUpper(), Json::string(format)});
    auto format = dialog.choice("videoFormat", t("videoFormat"), formats);
    if (!profile.has("videoFormat"))
        format->setCurrentIndex(4);
    dialog
        .choice("encoderQuality", t("encoderQuality"),
                {{t("encoderQualityAuto"), Json::string("auto")},
                 {t("encoderQualityLight"), Json::string("light")},
                 {t("encoderQualityBalanced"), Json::string("balanced")},
                 {t("encoderQualityHigh"), Json::string("high")}})
        ->setToolTip(t("encoderQualityHelp"));
    auto rate = dialog.number("videoBitrate", t("bitrate"), 4500, 100, 100000, 3);
    rate->setProperty("jsonScale", 1000.);
    rate->setRange(.1, 100);
    rate->setValue(profile.get("videoBitrate").num(4500) / 1000);
    rate->setSuffix(" Mbit/s");
    dialog.number("gopSize", t("gopSize"), 2, 1, 10)->setSuffix(" s");
    for (const auto& field : {"videoWidth", "videoHeight", "frameRate"}) {
        auto input = dialog.number(field,
                                   t(field == QString("videoWidth")    ? "width"
                                     : field == QString("videoHeight") ? "height"
                                                                       : "frameRate"),
                                   0, 0, field == QString("frameRate") ? 240 : 7680,
                                   field == QString("frameRate") ? 3 : 0);
        input->setProperty("nullable", true);
        input->setSpecialValueText(t("keepSource"));
        input->setSingleStep(field == QString("frameRate") ? 1 : 2);
    }
    dialog.grid({{"videoCodec", t("codec")},
                 {"videoCodecName", t("videoCodecName")},
                 {"encoderQuality", t("encoderQuality")},
                 {"pixelFormat", t("pixelFormat")},
                 {"videoFormat", t("videoFormat")},
                 {"videoBitrate", t("bitrate")},
                 {"gopSize", t("gopSize")},
                 {"videoWidth", t("width")},
                 {"videoHeight", t("height")},
                 {"frameRate", t("frameRate")}},
                4);
    dialog.section(t("audio") + " · " + t("additionalVideoOptions"));
    codec("audioCodec", "audioCodec", "AudioCodecs", {"aac"}, true);
    dialog.number("audioBitrate", t("audioBitrate"), 128, 32, 512)->setSuffix(" kbit/s");
    QList<QPair<QString, Json>> presetValues, tuneValues;
    for (const auto& name : {"ultrafast", "superfast", "veryfast", "faster", "fast", "medium",
                             "slow", "slower", "veryslow"})
        presetValues.append({name, Json::string(name)});
    for (const auto& name :
         {"zerolatency", "film", "animation", "grain", "stillimage", "fastdecode"})
        tuneValues.append({name, Json::string(name)});
    auto preset = dialog.choice("preset", "preset", presetValues);
    if (!profile.has("preset"))
        preset->setCurrentIndex(2);
    auto tune = dialog.choice("tune", "tune", tuneValues);
    dialog.grid({{"audioCodec", t("audioCodec")},
                 {"audioBitrate", t("audioBitrate")},
                 {"preset", "preset"},
                 {"tune", "tune"}},
                4);
    auto option = [&](const char* key, const char* help, bool fallback) {
        auto input = dialog.check(key, t(key), fallback, true);
        auto taken = dialog.form->takeRow(input);
        delete taken.fieldItem;
        auto card = new QFrame(&dialog);
        card->setObjectName("optionCard");
        auto row = new QHBoxLayout(card);
        auto text = new QVBoxLayout;
        auto title = new QLabel(t(key), card);
        title->setObjectName("rowTitle");
        text->addWidget(title);
        auto hint = new QLabel(t(help), card);
        hint->setObjectName("muted");
        hint->setWordWrap(true);
        text->addWidget(hint);
        row->addLayout(text, 1);
        input->setText({});
        row->addWidget(input);
        dialog.form->addRow(card);
        return input;
    };
    auto low = option("lowLatency", "lowLatencyHelp", true);
    option("adaptiveBitrate", "adaptiveBitrateHelp", true);
    auto tuning = [preset, tune](bool on) {
        preset->setEnabled(!on);
        tune->setEnabled(!on);
    };
    connect(low, &QCheckBox::toggled, &dialog, tuning);
    tuning(low->isChecked());
    for (const auto& platform : {"Twitch", "YouTube", "Kick", "Facebook"}) {
        auto button = new QPushButton(
            platform == QString("Facebook") ? "Facebook Gaming" : platform, presets);
        button->setIcon(platformMark(platform));
        button->setObjectName(QString("preset_") + platform);
        button->setProperty("brand", QString(platform).toLower());
        presetRow->addWidget(button);
        connect(button, &QPushButton::clicked, &dialog, [&, platform] {
            for (size_t i = 0; i < templates.size(); ++i) {
                auto record = templates.at(i);
                if (record.get("isDefaultConfiguration").flag() &&
                    str(record.get("platform")) == platform) {
                    for (const auto& key :
                         {"videoWidth", "videoHeight", "frameRate", "gopSize", "audioBitrate"})
                        dialog.findChild<QDoubleSpinBox*>(key)->setValue(record.get(key).num());
                    rate->setValue(record.get("videoBitrate").num(4500) / 1000);
                    low->setChecked(record.get("lowLatency").flag(true));
                    break;
                }
            }
        });
    }
    if (profile.has("id") && !apply) {
        auto remove = dialog.buttons->addButton(t("delete"), QDialogButtonBox::DestructiveRole);
        remove->setIcon(studioIcon("delete"));
        connect(remove, &QPushButton::clicked, &dialog, [&] {
            run([&] {
                if (confirm("deleteConfiguration")) {
                    studio.store.remove("profiles", profile.get("id").num());
                    dialog.reject();
                }
            });
        });
    }
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto result = dialog.value();
            result.set("isDefaultConfiguration", false);
            validate("profiles", result);
            if (apply)
                apply(result);
            else
                studio.store.save("profiles", result, profile.get("id").num());
            dialog.accept();
        });
    });
    if (apply)
        presets->hide();
    dialog.exec();
    sample();
}

// La selezione impone due passaggi come Windows e non rende disponibili configurazioni disattivate.
void NativeWindow::startWizard(bool playlist) {
    auto profiles = studio.store.list("profiles"), channels = studio.store.list("channels");
    QWizard wizard(this);
    wizard.setObjectName("startWizard");
    wizard.setWindowTitle(t(playlist ? "startPlaylistTitle" : "startNewLiveTitle"));
    wizard.setWizardStyle(QWizard::ModernStyle);
    wizard.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    wizard.resize(std::max(width(), screen()->availableGeometry().width()) - 28,
                  screen()->availableGeometry().height() - 28);
    auto pageHeading = [&](QFormLayout* form, const QString& step, QWidget* page) {
        form->setContentsMargins(16, 0, 16, 0);
        auto header = new QFrame(page);
        header->setObjectName("wizardHeading");
        auto row = new QHBoxLayout(header);
        row->setContentsMargins(0, 16, 0, 16);
        auto title = new QLabel(wizard.windowTitle(), header);
        title->setObjectName("dialogTitle");
        row->addWidget(title, 1);
        auto close = new QPushButton(header);
        close->setObjectName("closeWizard");
        close->setAutoDefault(false);
        close->setIcon(studioIcon("close"));
        close->setAccessibleName(t("close"));
        row->addWidget(close);
        connect(close, &QPushButton::clicked, &wizard, &QDialog::reject);
        form->addRow(header);
        if (!step.isEmpty()) {
            auto label = new QLabel(step, page);
            label->setStyleSheet("font-weight: 600; margin-top: 14px;");
            form->addRow(label);
        }
    };
    wizard.setOption(QWizard::NoBackButtonOnStartPage);
    wizard.setButtonText(QWizard::NextButton, t("next"));
    wizard.setButtonText(QWizard::BackButton, t("back"));
    wizard.setButtonText(QWizard::CancelButton, t("cancel"));
    wizard.setButtonText(QWizard::FinishButton,
                         t(playlist ? "startPlaylistButton" : "composeNext"));
    auto first = new ChoiceWizardPage;
    auto firstForm = new QFormLayout(first);
    pageHeading(firstForm, t("wizardStep1Title"), first);
    firstForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    auto profile = new ChoiceList(first);
    profile->setObjectName("wizardProfile");
    profile->setAccessibleName(t("videoAndAudio"));
    profile->setWordWrap(true);
    profile->setTextElideMode(Qt::ElideNone);
    for (auto entry : choices(profiles, "title", true)) {
        const auto value = studio.store.get("profiles", entry.second.num());
        QStringList details{QString::fromStdString(value.get("videoCodecName").str("–"))};
        if (value.get("videoWidth").num() > 0 && value.get("videoHeight").num() > 0)
            details << QString::number(value.get("videoWidth").num()) + "×" +
                           QString::number(value.get("videoHeight").num());
        if (value.get("frameRate").num() > 0)
            details << QString::number(value.get("frameRate").num(), 'g', 5) + " fps";
        details << QString::number(value.get("videoBitrate").num(4500) / 1000, 'g', 5) + " Mbit/s"
                << QString::fromStdString(value.get("videoFormat").str("flv")).toUpper()
                << "GOP " + QString::number(value.get("gopSize").num(2))
                << t("audioBitrate") + " " + QString::number(value.get("audioBitrate").num(128)) +
                       " kbit/s";
        auto item = new QListWidgetItem(profileTitle(value) + "\n" + details.join(" · "), profile);
        item->setSizeHint(QSize(310, 96));
        item->setData(Qt::UserRole, entry.second.num());
    }
    profile->setIconSize(QSize(28, 28));
    profile->setViewMode(QListView::IconMode);
    profile->setResizeMode(QListView::Adjust);
    profile->setMovement(QListView::Static);
    profile->setGridSize(QSize(330, 110));
    profile->setSpacing(8);
    for (int i = 0; i < profile->count(); ++i) {
        auto record =
            studio.store.get("profiles", profile->item(i)->data(Qt::UserRole).toLongLong());
        profile->item(i)->setIcon(platformMark(QString::fromStdString(
            record.get("platform").str(record.get("defaultPlatformConfiguration").str()))));
    }
    profile->setCurrentRow(0);
    decorateChoices(profile);
    first->complete = [profile] { return profile->currentItem() != nullptr; };
    connect(profile, &QListWidget::currentRowChanged, first, &ChoiceWizardPage::refresh);
    if (!profile->count()) {
        firstForm->addRow(new QLabel(t("wizardNoActiveSettingsMessage"), first));
        auto activate = new QPushButton(t("activateVideoAudioSettings"), first);
        firstForm->addRow(activate);
        connect(activate, &QPushButton::clicked, &wizard, [&] {
            wizard.reject();
            navigate("profiles");
        });
    }
    firstForm->addRow(profile);
    wizard.addPage(first);
    auto second = new ChoiceWizardPage;
    auto secondForm = new QFormLayout(second);
    pageHeading(secondForm, t("wizardStep2Title"), second);
    secondForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    auto channel = new ChoiceList(second);
    channel->setObjectName("wizardChannel");
    channel->setAccessibleName(t("channel"));
    channel->setWordWrap(true);
    channel->setTextElideMode(Qt::ElideNone);
    for (auto entry : choices(channels, "channelName", true)) {
        const auto value = studio.store.get("channels", entry.second.num());
        auto item = new QListWidgetItem(platformMark(str(value.get("platform"))),
                                        entry.first + " · " + str(value.get("platform")) + "\n" +
                                            str(value.get("streamUrl")) + "\n" +
                                            str(value.get("description")),
                                        channel);
        item->setSizeHint(QSize(310, 96));
        item->setData(Qt::UserRole, entry.second.num());
    }
    channel->setIconSize(QSize(28, 28));
    channel->setViewMode(QListView::IconMode);
    channel->setResizeMode(QListView::Adjust);
    channel->setMovement(QListView::Static);
    channel->setGridSize(QSize(330, 110));
    channel->setSpacing(8);
    channel->setCurrentRow(0);
    decorateChoices(channel);
    second->complete = [channel] { return channel->currentItem() != nullptr; };
    connect(channel, &QListWidget::currentRowChanged, second, &ChoiceWizardPage::refresh);
    if (!channel->count()) {
        secondForm->addRow(new QLabel(t("wizardNoActiveConfigurationsMessage"), second));
        auto activate = new QPushButton(t("activeConfigurationLabel"), second);
        secondForm->addRow(activate);
        connect(activate, &QPushButton::clicked, &wizard, [&] {
            wizard.reject();
            navigate("channels");
        });
    }
    secondForm->addRow(channel);
    auto title = new QLineEdit(second);
    title->setObjectName("wizardTitle");
    title->setPlaceholderText(t("titlePlaceholder"));
    title->hide();
    wizard.addPage(second);
    auto folder = new QLineEdit(&wizard);
    folder->hide();
    folder->setObjectName("wizardFolder");
    auto repeat = new QCheckBox(t("nativeLoop"), &wizard);
    repeat->hide();
    repeat->setObjectName("wizardLoop");
    auto countdown = new QDoubleSpinBox(&wizard);
    countdown->hide();
    countdown->setRange(0, 60);
    countdown->setDecimals(0);
    countdown->setValue(0);
    countdown->setObjectName("wizardCountdown");
    auto layoutPicker = new QComboBox(second);
    layoutPicker->addItem(t("blankLayout"), 0);
    auto scenes = studio.store.list("scenes");
    for (size_t i = 0; i < scenes.size(); ++i)
        if (scenes.at(i).get("isLayout").flag())
            layoutPicker->addItem(str(scenes.at(i).get("name")), scenes.at(i).get("id").num());
    layoutPicker->hide();
    if (playlist) {
        auto third = new ChoiceWizardPage;
        auto form = new QFormLayout(third);
        pageHeading(form, {}, third);
        auto hint = new QLabel(
            t("playlistOnlyVideos") + " MP4, MKV, MOV, AVI, WebM, M4V, TS, FLV, MPEG, MPG.", third);
        hint->setWordWrap(true);
        form->addRow(new QLabel(t("wizardStep1Title"), third));
        form->addRow(profile);
        profile->setMinimumHeight(220);
        profile->setMaximumHeight(240);
        if (!profile->count()) {
            form->addRow(new QLabel(t("wizardNoActiveSettingsMessage"), third));
            auto activate = new QPushButton(t("activateVideoAudioSettings"), third);
            form->addRow(activate);
            connect(activate, &QPushButton::clicked, &wizard, [&] {
                wizard.reject();
                navigate("profiles");
            });
        }
        form->addRow(new QLabel(t("wizardStep2Title"), third));
        form->addRow(channel);
        channel->setMinimumHeight(120);
        channel->setMaximumHeight(200);
        if (!channel->count()) {
            form->addRow(new QLabel(t("wizardNoActiveConfigurationsMessage"), third));
            auto activate = new QPushButton(t("activeConfigurationLabel"), third);
            form->addRow(activate);
            connect(activate, &QPushButton::clicked, &wizard, [&] {
                wizard.reject();
                navigate("channels");
            });
        }

        form->addRow(hint);
        form->addRow(t("playlistFolder"), folder);
        folder->show();
        repeat->hide();
        countdown->hide();
        auto choose = new QPushButton(t("playlistFolder") + "…", third);
        form->addRow(choose);
        connect(choose, &QPushButton::clicked, &wizard, [&] {
            auto path = QFileDialog::getExistingDirectory(&wizard, t("playlistFolder"));
            if (!path.isEmpty())
                folder->setText(path);
        });
        wizard.removePage(0);
        wizard.removePage(1);
        const auto id = wizard.addPage(third);
        wizard.setStartId(id);
        profile->show();
        channel->show();
        const int columns = std::max(1, (wizard.width() - 60) / 330);
        profile->setFixedHeight(
            std::clamp(((profile->count() + columns - 1) / columns) * 110 + 12, 110, 240));
        channel->setFixedHeight(
            std::clamp(((channel->count() + columns - 1) / columns) * 110 + 12, 110, 200));
        third->complete = [profile, channel, folder] {
            return profile->currentItem() && channel->currentItem() &&
                   !folder->text().trimmed().isEmpty();
        };
        connect(folder, &QLineEdit::textChanged, third, &ChoiceWizardPage::refresh);
    }
    // La conferma resta aperta finché i dati minimi sono validi.
    disconnect(wizard.button(QWizard::FinishButton), nullptr, &wizard, nullptr);
    for (auto which :
         {QWizard::FinishButton, QWizard::NextButton, QWizard::BackButton, QWizard::CancelButton}) {
        auto button = wizard.button(which);
        button->setMinimumWidth(button->fontMetrics().horizontalAdvance(button->text()) + 40);
    }
    connect(wizard.button(QWizard::FinishButton), &QAbstractButton::clicked, &wizard, [&] {
        run([&] {
            if (playlist && scanVideos(folder->text().toStdString()).empty())
                throw std::runtime_error("Nessun video");
            wizard.accept();
        });
    });
    bool sceneStarted = false;
    do {
        if (wizard.exec() != QDialog::Accepted)
            return;
        Json request;
        request.set("profileId", profile->currentItem()->data(Qt::UserRole).toDouble());
        request.set("channelId", channel->currentItem()->data(Qt::UserRole).toDouble());
        request.set("title", title->text().isEmpty() ? t("untitled").toStdString()
                                                     : title->text().toStdString());
        if (playlist) {
            request.set("loop", repeat->isChecked());
            if (countdown->value() > 0) {
                QDialog pending(this);
                pending.setObjectName("streamCountdown");
                pending.setWindowTitle(t("upcomingLive"));
                auto layout = new QVBoxLayout(&pending);
                auto label = new QLabel(&pending);
                layout->addWidget(label);
                auto cancel = new QDialogButtonBox(QDialogButtonBox::Cancel, &pending);
                layout->addWidget(cancel);
                connect(cancel, &QDialogButtonBox::rejected, &pending, &QDialog::reject);
                int left = countdown->value();
                label->setText(t("upcomingLive") + " · " + QLocale().toString(left) + " s");
                QTimer timer;
                connect(&timer, &QTimer::timeout, &pending, [&] {
                    if (--left <= 0)
                        pending.accept();
                    else
                        label->setText(t("upcomingLive") + " · " + QLocale().toString(left) + " s");
                });
                timer.start(1000);
                if (pending.exec() != QDialog::Accepted)
                    return;
            }
            auto files = Json::array();
            for (const auto& file : scanVideos(folder->text().toStdString()))
                files.push(Json::string(file));
            request.set("files", files);
            runAsync([this, request] { return Json::number(studio.start(request)); },
                     [this](Json id) {
                         watched = id.num();
                         navigate("live");
                     });
        } else {
            Json scene;
            if (layoutPicker->currentData().toLongLong()) {
                scene =
                    studio.store.get("scenes", layoutPicker->currentData().toLongLong()).clone();
                scene.set("id", 0.0);
            }
            scene.set("isLayout", false);
            const bool back = editScene(scene, [this, request, &sceneStarted](Json scene) mutable {
                sceneStarted = true;
                request.set("scene", scene);
                runAsync([this, request] { return Json::number(studio.start(request)); },
                         [this](Json id) {
                             watched = id.num();
                             navigate("live");
                         });
            });
            if (!back && !sceneStarted)
                return;
        }
    } while (!playlist && !sceneStarted);
}

// Le scene del deck diventano gli stessi comandi takeover usati dall'API; il live non cambia
// identità.
void NativeWindow::liveCommand(long id, const std::string& action, Json parameters) {
    auto session = studio.session(id);
    if (!session)
        throw std::runtime_error("Live non disponibile");
    if (action == "button") {
        auto button = studio.store.get("buttons", parameters.get("id").num());
        auto media = studio.store.root / "scene-media" / button.get("mediaName").str();
        if (media.filename() != button.get("mediaName").str() || !fs::is_regular_file(media))
            throw std::runtime_error("File non disponibile");
        Json scene, item;
        scene.set("name", button.get("label"));
        const bool overlay = button.get("displayMode").str() == "in_scene";
        item.set("sourceKind", overlay ? "OVERLAY" : "FILE");
        item.set("sourceTarget", media.string());
        item.set("audioEnabled", true);
        auto ext = media.extension().string();
        item.set("loop", ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" ||
                             ext == ".webp");
        if (overlay) {
            for (auto field : {"x", "y", "width", "height"})
                if (button.has(field))
                    item.set(field, button.get(field));
            auto snap = session->snapshot();
            double w = snap.get("scene").get("width").num(
                       snap.get("profile").get("videoWidth").num(1280)),
                   h = snap.get("scene").get("height").num(
                       snap.get("profile").get("videoHeight").num(720)),
                   iw = button.get("width").num(320), ih = button.get("height").num(180);
            auto place = button.get("placement").str("custom");
            if (place != "custom") {
                item.set("x", place == "center"                          ? (w - iw) / 2
                              : place.find("right") != std::string::npos ? w - iw
                                                                         : 0);
                item.set("y", place == "center"                           ? (h - ih) / 2
                              : place.find("bottom") != std::string::npos ? h - ih
                                                                          : 0);
            }
        }
        auto items = Json::array();
        items.push(item);
        scene.set("items", items);
        button.set("scene", scene);
        session->command("takeover", button);
    } else
        session->command(action, parameters);
}

// Riusa lo snapshot storico, ma Studio risolve di nuovo canale e chiave per evitare credenziali
// obsolete.
void NativeWindow::replay(Json history, bool beginning, int index) {
    if (!confirm(beginning ? "restartLiveAlert" : "startLiveAlert"))
        return;
    auto request = history.get("request").clone();
    request.set("offsetSeconds",
                beginning ? 0 : history.get("offsetSeconds").num(history.get("offset").num()));
    request.set("startIndex", index >= 0 ? index : beginning ? 0 : history.get("position").num());
    runAsync(
        [this, request, history] {
            const auto live = studio.live();
            for (size_t i = 0; i < live.size(); ++i)
                if (live.at(i).get("id").num() == history.get("id").num())
                    studio.session(history.get("id").num())->stop();
            return Json::number(studio.start(request));
        },
        [this](Json id) {
            watched = id.num();
            navigate("preview");
        });
}

// Aggiorna il profilo attivo e quello da riusare nei riavvii, senza alterare le altre richieste
// storiche.
void NativeWindow::linkProfile(Json history) {
    auto profile = history.get("request").get("profile");
    if (!profile.has("title"))
        profile = studio.store.get("profiles", history.get("request").get("profileId").num(1));
    editProfile(profile, [this, history](Json profile) {
        const long id = history.get("id").num();
        const auto sessions = studio.live();
        for (size_t i = 0; i < sessions.size(); ++i)
            if (sessions.at(i).get("id").num() == id && !studio.session(id)->done())
                studio.session(id)->command("parameters", profile);
        auto stored = studio.store.get("history", id);
        auto request = stored.get("request").clone();
        request.set("profile", profile);
        stored.set("request", request);
        studio.store.save("history", stored, id);
    });
}

// Lo snapshot resta leggibile come dati di diagnostica in un controllo di testo nativo.
void NativeWindow::showSnapshot(const Json& object) {
    QDialog dialog(this);
    dialog.setWindowTitle(t("seeDetails"));
    auto layout = new QVBoxLayout(&dialog);
    auto text = new QPlainTextEdit(QString::fromStdString(object.dump()), &dialog);
    text->setReadOnly(true);
    layout->addWidget(text);
    auto close = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(close, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(close);
    dialog.resize(760, 520);
    dialog.exec();
}

void NativeWindow::showLiveDetails(Json history) {
    QDialog dialog(this);
    dialog.setObjectName("liveDetailsDialog");
    const bool scene = history.get("request").has("scene");
    dialog.setWindowTitle(t(scene ? "sceneDetails" : "playlistDetails"));
    dialog.resize(std::max(800, std::min(1300, width() - 50)), 620);
    auto layout = new QVBoxLayout(&dialog);
    auto heading = new QLabel(&dialog);
    heading->setObjectName("dialogTitle");
    layout->addWidget(heading);
    auto caption = new QLabel(&dialog);
    caption->setObjectName("muted");
    layout->addWidget(caption);
    auto table = new QTableWidget(0, scene ? 3 : 5, &dialog);
    table->setObjectName("playlistVideos");
    table->setHorizontalHeaderLabels(
        scene ? QStringList{"#", t("liveStatus"), t("title")}
              : QStringList{"#", t("liveStatus"), t("title"), t("playlistStoppedAt"), ""});
    table->verticalHeader()->hide();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    layout->addWidget(table, 1);
    auto logs = new QPlainTextEdit(&dialog);
    logs->setObjectName("liveLogs");
    logs->setReadOnly(true);
    logs->setLineWrapMode(QPlainTextEdit::NoWrap);
    logs->hide();
    auto toggle = new QPushButton("FFmpeg · logs", &dialog);
    toggle->setAutoDefault(false);
    toggle->setCheckable(true);
    layout->addWidget(toggle);
    layout->addWidget(logs, 1);
    connect(toggle, &QPushButton::toggled, logs, &QWidget::setVisible);
    auto refresh = [&, history] {
        Json record;
        try {
            record = studio.store.get("history", history.get("id").num());
        } catch (...) {
            heading->setText(t("noDataPresent"));
            return;
        }
        const auto sessions = studio.live();
        bool active = false;
        for (size_t i = 0; i < sessions.size(); ++i)
            if (sessions.at(i).get("id").num() == history.get("id").num()) {
                const auto snap = sessions.at(i);
                record.set("position", snap.get("position"));
                record.set("videoResults", snap.get("videoResults"));
                record.set("status", snap.get("status"));
                active = snap.get("status").str() == "LIVE" ||
                         snap.get("status").str() == "STARTING" ||
                         snap.get("status").str() == "RECONNECTING";
            }
        auto items = scene ? record.get("request").get("scene").get("items")
                           : record.get("request").get("files");
        heading->setText(
            (scene ? str(record.get("request").get("scene").get("name")) : t("playlistDetails")) +
            " · " +
            t(scene ? "sceneSources" : "playlistVideoCount")
                .replace("{0}", QString::number(items.size())));
        caption->setText(str(record.get("platform")) + " · " + str(record.get("channelName")));
        table->setRowCount(0);
        for (size_t i = 0; i < items.size(); ++i) {
            table->insertRow(i);
            auto result = record.get("videoResults").at(scene ? 0 : i);
            const auto status = result.get("status").str(record.get("status").str());
            auto statusKey = status == "LIVE"           ? "liveStatusLive"
                             : status == "ERROR"        ? "liveStatusError"
                             : status == "ENDED"        ? "liveStatusEnded"
                             : status == "STOPPED"      ? "liveStatusStopped"
                             : status == "STARTING"     ? "liveStatusConnecting"
                             : status == "RECONNECTING" ? "liveStatusReconnecting"
                                                        : "liveStatusOffline";
            const auto path = scene ? items.at(i).get("sourceTarget").str() : items.at(i).str();
            table->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
            auto statusCell = new QWidget(table);
            auto statusRow = new QHBoxLayout(statusCell);
            statusRow->setContentsMargins(8, 0, 8, 0);
            auto badge = new QLabel(t(statusKey), statusCell);
            badge->setFixedHeight(20);
            statusRow->addWidget(badge, 0, Qt::AlignVCenter);
            statusRow->addStretch();
            badge->setProperty("badge", status == "STARTING" || status == "RECONNECTING"
                                            ? "connecting"
                                            : QString::fromStdString(status).toLower());
            table->setCellWidget(i, 1, statusCell);
            auto title = new QTableWidgetItem(
                QString::fromStdString(
                    scene ? items.at(i).get("label").str(fs::path(path).filename().string())
                          : fs::path(path).filename().string()) +
                "\n" + QString::fromStdString(path));
            title->setToolTip(QString::fromStdString(path));
            table->setItem(i, 2, title);
            table->setRowHeight(i, 54);
            if (!scene) {
                const int seconds = result.get("offsetSeconds").num();
                table->setItem(
                    i, 3,
                    new QTableWidgetItem(seconds > 0 && status != "ENDED"
                                             ? QString("%1:%2:%3")
                                                   .arg(seconds / 3600, 2, 10, QChar('0'))
                                                   .arg(seconds / 60 % 60, 2, 10, QChar('0'))
                                                   .arg(seconds % 60, 2, 10, QChar('0'))
                                             : "–"));
                auto actions = new QWidget(table);
                auto row = new QHBoxLayout(actions);
                row->setContentsMargins(0, 0, 0, 0);
                if (status == "ERROR") {
                    auto report = new QToolButton(actions);
                    report->setObjectName("reportError");
                    report->setProperty("rowAction", true);
                    report->setFixedSize(32, 32);
                    report->setIconSize(QSize(18, 18));
                    report->setIcon(studioIcon("report"));
                    report->setToolTip(t("reportError"));
                    row->addWidget(report);
                    connect(report, &QToolButton::clicked, &dialog, [this, record, result, i] {
                        auto report = record.clone();
                        report.set("error", result.get("error"));
                        showErrorReport(report, i);
                    });
                }
                auto action = [&](const char* name, const char* icon, QColor color, bool enabled,
                                  std::function<void()> handler) {
                    auto control = new QToolButton(actions);
                    control->setObjectName(name);
                    control->setProperty("rowAction", true);
                    control->setFixedSize(32, 32);
                    control->setIconSize(QSize(18, 18));
                    control->setIcon(studioIcon(icon, color));
                    control->setToolTip(t(name));
                    control->setAccessibleName(t(name));
                    control->setEnabled(enabled);
                    row->addWidget(control);
                    connect(control, &QToolButton::clicked, &dialog, std::move(handler));
                };
                const auto platform = str(record.get("platform"));
                if (status == "LIVE" &&
                    QStringList{"Twitch", "YouTube", "Kick", "Facebook"}.contains(platform)) {
                    auto publicPage = new QToolButton(actions);
                    publicPage->setProperty("rowAction", true);
                    publicPage->setFixedSize(32, 32);
                    publicPage->setIconSize(QSize(18, 18));
                    publicPage->setIcon(platformMark(platform));
                    publicPage->setToolTip(platform == "Facebook" ? "Facebook Gaming" : platform);
                    row->addWidget(publicPage);
                    connect(publicPage, &QToolButton::clicked, &dialog, [this, record] {
                        runAsync(
                            [record] {
                                auto channel = record.get("request").clone();
                                channel.set("platform", record.get("platform"));
                                channel.set("channelName", record.get("channelName"));
                                return platformPlayer(channel);
                            },
                            [](Json player) {
                                const auto url =
                                    QString::fromStdString(player.get("publicUrl").str());
                                if (!url.isEmpty())
                                    QDesktopServices::openUrl(QUrl(url));
                            });
                    });
                }
                if (status == "LIVE") {
                    action("preview", "preview", {}, true, [this, record, &dialog] {
                        watched = record.get("id").num();
                        dialog.accept();
                        navigate("preview");
                    });
                }
                action("playSpot", "spot", {}, status == "LIVE",
                       [this, record] { playSpot(record.get("id").num()); });
                action("stopLiveIconLabel", "stop", QColor("#ff453a"), active, [this, record] {
                    if (confirm("stopLive"))
                        runAsync([this, record] {
                            if (auto session = studio.session(record.get("id").num()))
                                session->stop();
                            return Json{};
                        });
                });
                auto start = new QToolButton(actions);
                start->setObjectName("playlistStartFrom");
                start->setProperty("rowAction", true);
                start->setFixedSize(32, 32);
                start->setIconSize(QSize(18, 18));
                start->setIcon(studioIcon("play", QColor("#30d158")));
                start->setToolTip(t("playlistStartFrom"));
                start->setEnabled(!active);
                row->addWidget(start);
                connect(start, &QToolButton::clicked, &dialog, [this, record, i, &dialog] {
                    dialog.accept();
                    replay(record, true, i);
                });
                action("restartLiveIconLabel", "restart", QColor("#ffd60a"), true,
                       [this, record, i, &dialog] {
                           dialog.accept();
                           replay(record, true, i);
                       });
                action("deleteLiveRow", "delete", QColor("#ff453a"), !active, [this, record, i] {
                    run([&] {
                        if (!confirm("deleteLiveRowConfirm"))
                            return;
                        auto updated = studio.store.get("history", record.get("id").num());
                        auto request = updated.get("request").clone();
                        auto files = Json::array(), results = Json::array(),
                             profiles = Json::array();
                        for (size_t n = 0; n < request.get("files").size(); ++n)
                            if (n != i) {
                                files.push(request.get("files").at(n));
                                results.push(updated.get("videoResults").at(n));
                                profiles.push(request.get("videoProfiles").at(n));
                            }
                        if (!files.size()) {
                            studio.store.removeHistoryIfInactive(updated.get("id").num(),
                                                                 updated.get("startedAt").str());
                            return;
                        }
                        request.set("files", files);
                        request.set("videoProfiles", profiles);
                        updated.set("request", request);
                        updated.set("videoResults", results);
                        updated.set("position", 0.);
                        studio.store.save("history", updated, updated.get("id").num());
                    });
                });
                table->setCellWidget(i, 4, actions);
            }
        }
        if (logs->isVisible()) {
            const auto content =
                QString::fromStdString(readStreamLogs(studio.store.root, record.get("id").num()));
            if (logs->toPlainText() != content)
                logs->setPlainText(content);
        }
    };
    refresh();
    auto timer = new QTimer(&dialog);
    connect(timer, &QTimer::timeout, &dialog, refresh);
    timer->start(1000);
    auto close = new QPushButton(t("back"), &dialog);
    close->setObjectName("closeLiveDetails");
    layout->addWidget(close, 0, Qt::AlignRight);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

// Il mixer usa percentuali: muting e guadagno sono applicati dal motore alla stessa scena in onda.
void NativeWindow::showMixer(long id) {
    auto session = studio.session(id);
    if (!session)
        return;
    auto snap = session->snapshot();
    auto items = snap.get("scene").get("items");
    if (!items.isArray())
        items = snap.get("request").get("scene").get("items");
    QDialog dialog(this);
    dialog.setWindowTitle(t("sourceVolumes"));
    auto form = new QFormLayout(&dialog);
    for (size_t i = 0; i < items.size(); ++i) {
        auto row = new QWidget(&dialog);
        auto horizontal = new QHBoxLayout(row);
        auto volume = new QSlider(Qt::Horizontal, row);
        volume->setRange(0, 200);
        volume->setValue(items.at(i).get("volume").num(100));
        auto mute = new QCheckBox(t("volumeMute"), row);
        mute->setChecked(!items.at(i).get("audioEnabled").flag(true));
        horizontal->addWidget(volume);
        horizontal->addWidget(mute);
        form->addRow(str(items.at(i).get("sourceTarget")), row);
        auto apply = [this, id, i, volume, mute] {
            run([&] {
                Json command;
                command.set("index", double(i));
                command.set("volume", double(volume->value()));
                command.set("audioEnabled", !mute->isChecked());
                liveCommand(id, "volume", command);
            });
        };
        connect(volume, &QSlider::sliderReleased, &dialog, apply);
        connect(mute, &QCheckBox::toggled, &dialog, apply);
    }
    dialog.resize(600, 350);
    dialog.exec();
}

// Aprire il deck arma i tasti per questo live nelle schermate Live/Anteprima, sempre con conferma.
void NativeWindow::showDeck(long id) {
    armedDeck = 0;
    const auto sessions = studio.live();
    for (size_t i = 0; i < sessions.size(); ++i)
        if (sessions.at(i).get("id").num() == id && sessions.at(i).get("status").str() == "LIVE")
            armedDeck = id;
    QDialog dialog(this);
    dialog.setObjectName("sceneDeckDialog");
    dialog.setWindowTitle(t("sceneDeck"));
    dialog.resize(900, 720);
    auto layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    auto title = new QLabel(t("sceneDeck"), &dialog);
    title->setObjectName("dialogTitle");
    layout->addWidget(title);
    auto offline = new QLabel(t("sceneNotLive"), &dialog);
    offline->setObjectName("sceneDeckOffline");
    layout->addWidget(offline);
    auto resume = new QPushButton(t("sceneResume"), &dialog);
    resume->setObjectName("sceneResume");
    resume->setIcon(studioIcon("play"));
    layout->addWidget(resume);
    connect(resume, &QPushButton::clicked, &dialog,
            [this, id] { run([&] { liveCommand(id, "resume"); }); });
    auto hint = new QLabel(t("sceneDeckHint"), &dialog);
    hint->setWordWrap(true);
    hint->setObjectName("muted");
    layout->addWidget(hint);
    auto stack = new QStackedWidget(&dialog);
    auto scroll = new QScrollArea(stack);
    scroll->setWidgetResizable(true);
    auto deck = new QWidget(scroll);
    auto grid = new QGridLayout(deck);
    scroll->setWidget(deck);
    stack->addWidget(scroll);
    auto editor = new QWidget(stack);
    editor->setObjectName("sceneDeckEditorHost");
    stack->addWidget(editor);
    layout->addWidget(stack, 1);
    QList<QShortcut*> localKeys;
    std::function<void()> render;
    auto edit = [&](Json button) {
        stack->setCurrentWidget(editor);
        hint->hide();
        editButton(button, editor);
        stack->setCurrentWidget(scroll);
        hint->show();
        render();
    };
    render = [&] {
        for (auto shortcut : localKeys)
            delete shortcut;
        localKeys.clear();
        for (auto shortcut : deckShortcuts)
            delete shortcut;
        deckShortcuts.clear();
        while (auto item = grid->takeAt(0)) {
            if (auto widget = item->widget()) {
                widget->hide();
                widget->deleteLater();
            }
            delete item;
        }
        const auto rows = studio.store.list("buttons");
        for (size_t i = 0; i < rows.size(); ++i) {
            const auto button = rows.at(i);
            auto card = new QWidget(deck);
            auto column = new QVBoxLayout(card);
            auto tile = new SceneTile(
                str(button.get("label")),
                QString::fromStdString(
                    (studio.store.root / "scene-media" / button.get("mediaName").str()).string()),
                str(button.get("hotkey")),
                t(button.get("displayMode").str() == "in_scene" ? "sceneModeInScene"
                                                                : "sceneModeFullscreen"),
                card);
            tile->setObjectName("sceneDeckTile");
            tile->setProperty("label", str(button.get("label")));
            column->addWidget(tile);
            auto change = new QPushButton(t("sceneEdit"), card);
            change->setObjectName("editSceneButton");
            change->setIcon(studioIcon("edit"));
            column->addWidget(change);
            connect(change, &QPushButton::clicked, &dialog, [&, button] { edit(button); });
            auto activate = [this, id, button] {
                const auto sessions = studio.live();
                bool live = false;
                for (size_t i = 0; i < sessions.size(); ++i)
                    if (sessions.at(i).get("id").num() == id)
                        live = sessions.at(i).get("status").str() == "LIVE";
                if (!live)
                    return;
                if (confirm(button.get("displayMode").str() == "in_scene" ? "sceneConfirmInScene"
                                                                          : "sceneConfirm",
                            str(button.get("label"))))
                    run([&] {
                        Json command;
                        command.set("id", button.get("id"));
                        liveCommand(id, "button", command);
                    });
            };
            connect(tile, &QPushButton::clicked, &dialog, activate);
            grid->addWidget(card, int(i) / 3, int(i) % 3);
            auto key = str(button.get("hotkey"));
            key.replace("Key", "");
            key.replace("Digit", "");
            if (!key.isEmpty()) {
                auto shortcut = new QShortcut(QKeySequence(key), &dialog);
                shortcut->setContext(Qt::WidgetWithChildrenShortcut);
                connect(shortcut, &QShortcut::activated, &dialog, [stack, scroll, activate] {
                    if (stack->currentWidget() == scroll)
                        activate();
                });
                localKeys.append(shortcut);
                auto armed = new QShortcut(QKeySequence(key), this);
                armed->setContext(Qt::WindowShortcut);
                connect(armed, &QShortcut::activated, this, [this, id, activate] {
                    if (armedDeck == id && (page == "live" || page == "preview"))
                        activate();
                });
                deckShortcuts.append(armed);
            }
        }
        if (rows.size() < 6) {
            auto add = new QPushButton(t("sceneAdd"), deck);
            add->setObjectName("sceneAdd");
            add->setIcon(studioIcon("plus"));
            add->setMinimumHeight(145);
            grid->addWidget(add, int(rows.size()) / 3, int(rows.size()) % 3);
            connect(add, &QPushButton::clicked, &dialog, [&] { edit(Json{}); });
        }
    };
    auto sync = [&] {
        Json snapshot;
        const auto sessions = studio.live();
        for (size_t i = 0; i < sessions.size(); ++i)
            if (sessions.at(i).get("id").num() == id)
                snapshot = sessions.at(i);
        const bool live = snapshot.get("status").str() == "LIVE";
        offline->setVisible(!live);
        resume->setVisible(live && snapshot.get("takeoverHolds").flag());
        for (auto tile : deck->findChildren<QPushButton*>("sceneDeckTile")) {
            tile->setEnabled(live);
            tile->setProperty("onAir", live && tile->property("label").toString() ==
                                                   str(snapshot.get("takeoverLabel")));
            tile->update();
        }
    };
    render();
    sync();
    auto timer = new QTimer(&dialog);
    connect(timer, &QTimer::timeout, &dialog, sync);
    timer->start(500);
    auto footer = new QHBoxLayout;
    auto keys = new QLabel(t("sceneKeysHint"), &dialog);
    keys->setWordWrap(true);
    footer->addWidget(keys, 1);
    auto disarm = new QPushButton(t("sceneKeysOff"), &dialog);
    footer->addWidget(disarm);
    connect(disarm, &QPushButton::clicked, &dialog, [this] { armedDeck = 0; });
    auto close = new QPushButton(t("close"), &dialog);
    close->setObjectName("closeSceneDeck");
    footer->addWidget(close);
    connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);
    layout->addLayout(footer);
    dialog.exec();
}

// Le esportazioni sono JSON privati e la reimportazione passa dalla validazione transazionale
// esistente.
void NativeWindow::exportData() {
    auto file = QFileDialog::getSaveFileName(this, t("save"), "orbis-stream.json", "JSON (*.json)");
    if (file.isEmpty())
        return;
    writeFile(file.toStdString(), studio.store.exportData().dump());
}

void NativeWindow::importData() {
    auto file = QFileDialog::getOpenFileName(this, t("configuration"), {}, "JSON (*.json)");
    if (file.isEmpty())
        return;
    studio.store.importData(Json::parse(readFile(file.toStdString())));
    sample();
}

void NativeWindow::removeRecord(const QString& kind, long id) {
    if (!confirm("deleteConfiguration"))
        return;
    if (kind == "scenes")
        ensureSceneCanDelete(studio.store, id);
    if (kind == "history") {
        auto record = studio.store.get("history", id);
        if (!studio.store.removeHistoryIfInactive(id, record.get("startedAt").str()))
            throw std::runtime_error("Live attiva");
    } else
        studio.store.remove(kind.toStdString(), id);
    sample();
}

// I record video restano collegabili a profili e riproducibili senza aprire pagine web.
void NativeWindow::editVideo(Json video) {
    NativeForm dialog(this, t("videoSettings"), video);
    dialog.text("name", t("title"));
    auto path = dialog.text("videoPath", t("path"));
    auto browse = new QPushButton(t("sceneMediaChoose"), &dialog);
    dialog.form->addRow(browse);
    connect(browse, &QPushButton::clicked, &dialog, [&] {
        auto file = QFileDialog::getOpenFileName(&dialog, t("video"));
        if (!file.isEmpty())
            path->setText(file);
    });
    dialog.check("enabled", t("active"), true);
    dialog.number("order", t("playlistPosition").replace("{0}", "").replace("{1}", ""), 0, 0,
                  100000);
    dialog.number("offsetSeconds", t("playlistStoppedAt") + " (s)", 0, 0, 864000, 2);
    dialog.choice("channelId", t("channel"), choices(studio.store.list("channels"), "channelName"));
    dialog.choice("profileId", t("videoAndAudio"), choices(studio.store.list("profiles"), "title"));
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            studio.store.save("videos", dialog.value(), video.get("id").num());
            dialog.accept();
        });
    });
    dialog.exec();
}

// Sei posizioni e chiavi uniche mantengono il contratto del deck originale, inclusi gli overlay
// temporizzati.
void NativeWindow::editButton(Json button, QWidget* host) {
    if (!button.has("id")) {
        auto all = studio.store.list("buttons");
        int free = -1;
        for (int slot = 0; slot < 6; ++slot) {
            bool occupied = false;
            for (size_t i = 0; i < all.size(); ++i)
                if (all.at(i).get("position").num() == slot)
                    occupied = true;
            if (!occupied) {
                free = slot;
                break;
            }
        }
        if (free < 0) {
            QMessageBox::information(this, t("warning"), t("sceneLimit"));
            return;
        }
        button.set("position", double(free));
    }
    NativeForm dialog(host ? host : this, t(button.has("id") ? "sceneEdit" : "sceneAdd"), button);
    if (host) {
        dialog.setWindowFlags(Qt::Widget);
        dialog.setMinimumWidth(0);
        if (!host->layout())
            new QVBoxLayout(host);
        host->layout()->setContentsMargins(0, 0, 0, 0);
        host->layout()->addWidget(&dialog);
    }
    dialog.text("label", t("sceneButtonName"));
    auto position = dialog.number("position", t("scenePlacement"), 0, 0, 5);
    position->hide();
    dialog.form->labelForField(position)->hide();
    auto key = dialog.text("hotkey", t("sceneKey"));
    key->setReadOnly(true);
    auto capture = new HotkeyCapture(t("sceneKeySet"), &dialog);
    capture->setObjectName("captureHotkey");
    capture->captured = [&, capture](QString value) {
        key->setText(value);
        capture->setText(t("sceneKeySet"));
    };
    connect(capture, &QPushButton::clicked, &dialog, [&, capture] {
        capture->setProperty("capturing", true);
        capture->setText(t("sceneKeyPress"));
        capture->setFocus();
    });
    dialog.form->addRow(capture);
    auto clear = new QPushButton(t("sceneKeyClear"), &dialog);
    connect(clear, &QPushButton::clicked, &dialog, [key] { key->clear(); });
    dialog.form->addRow(clear);
    auto media = new QLineEdit(str(button.get("mediaName")), &dialog);
    media->setReadOnly(true);
    dialog.form->addRow(t("sceneMedia"), media);
    auto choose = new QPushButton(t("sceneMediaChoose"), &dialog);
    dialog.form->addRow(choose);
    QString upload;
    connect(choose, &QPushButton::clicked, &dialog, [&] {
        auto file = QFileDialog::getOpenFileName(
            &dialog, t("sceneMedia"), {},
            "Media (*.mp4 *.mov *.mkv *.webm *.png *.jpg *.jpeg *.gif *.webp)");
        if (!file.isEmpty()) {
            upload = file;
            media->setText(file);
        }
    });
    dialog.setObjectName("sceneButtonEditor");
    auto mode = dialog.choice("displayMode", t("sceneDisplayMode"),
                              {{t("sceneModeFullscreen"), Json::string("fullscreen")},
                               {t("sceneModeInScene"), Json::string("in_scene")}});
    QList<QPair<QString, Json>> placements;
    for (auto p : {"bottomRight", "bottomLeft", "topRight", "topLeft", "center", "custom"}) {
        QString title = QString(p);
        title[0] = title[0].toUpper();
        placements.append({t(("scenePlacement" + title).toUtf8().constData()),
                           Json::string(p == std::string("bottomRight")  ? "bottom_right"
                                        : p == std::string("bottomLeft") ? "bottom_left"
                                        : p == std::string("topRight")   ? "top_right"
                                        : p == std::string("topLeft")    ? "top_left"
                                                                         : p)});
    }
    auto placement = dialog.choice("placement", t("scenePlacement"), placements);
    auto duration = dialog.number("durationSeconds", t("sceneDurationSeconds"), 0, 0, 86400);
    auto indefinite = new QCheckBox(t("sceneDurationForever"), &dialog);
    indefinite->setObjectName("sceneDurationForever");
    indefinite->setChecked(duration->value() == 0);
    dialog.form->addRow(indefinite);
    for (auto field : {"x", "y", "width", "height"})
        dialog.number(field,
                      field == std::string("width")    ? t("width")
                      : field == std::string("height") ? t("height")
                                                       : QString(field),
                      field == std::string("width")    ? 320
                      : field == std::string("height") ? 180
                                                       : 0,
                      0, 7680);
    dialog.grid({{"label", t("sceneButtonName")},
                 {"hotkey", t("sceneKey")},
                 {"displayMode", t("sceneDisplayMode")},
                 {"placement", t("scenePlacement")},
                 {"durationSeconds", t("sceneDuration")}},
                2);
    auto coordinates =
        dialog.grid({{"x", "X"}, {"y", "Y"}, {"width", t("width")}, {"height", t("height")}}, 4);
    auto sync = [mode, placement, duration, indefinite, coordinates] {
        const bool overlay =
            Json::parse(mode->currentData().toString().toStdString()).str() == "in_scene";
        placement->parentWidget()->setVisible(overlay);
        duration->parentWidget()->setVisible(overlay);
        indefinite->setVisible(overlay);
        duration->setEnabled(!indefinite->isChecked());
        coordinates->setVisible(
            overlay &&
            Json::parse(placement->currentData().toString().toStdString()).str() == "custom");
    };
    connect(mode, &QComboBox::currentIndexChanged, &dialog, sync);
    connect(placement, &QComboBox::currentIndexChanged, &dialog, sync);
    connect(indefinite, &QCheckBox::toggled, &dialog, sync);
    sync();
    if (button.has("id")) {
        auto remove =
            dialog.buttons->addButton(t("sceneDelete"), QDialogButtonBox::DestructiveRole);
        remove->setIcon(studioIcon("delete"));
        connect(remove, &QPushButton::clicked, &dialog, [&] {
            if (confirm("sceneDeleteConfirm")) {
                studio.store.remove("buttons", button.get("id").num());
                dialog.accept();
            }
        });
    }
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto value = dialog.value();
            if (indefinite->isChecked())
                value.set("durationSeconds", 0.);
            validate("buttons", value);
            auto all = studio.store.list("buttons");
            for (size_t i = 0; i < all.size(); ++i)
                if (all.at(i).get("id").num() != button.get("id").num() &&
                    (all.at(i).get("position").num() == value.get("position").num() ||
                     (!value.get("hotkey").str().empty() &&
                      all.at(i).get("hotkey").str() == value.get("hotkey").str())))
                    throw std::runtime_error("Posizione o scorciatoia già occupata");
            if (!upload.isEmpty())
                value.set("mediaName", copyMedia(studio.store.root / "scene-media", upload));
            if (value.get("mediaName").str().empty())
                throw std::runtime_error("Media richiesto");
            studio.store.save("buttons", value, button.get("id").num());
            dialog.accept();
        });
    });
    if (host) {
        // The Windows deck replaces its grid with the editor inside the same dialog.
        QEventLoop editing;
        connect(&dialog, &QDialog::finished, &editing, &QEventLoop::quit);
        if (auto deckDialog = qobject_cast<QDialog*>(host->window()))
            connect(deckDialog, &QDialog::finished, &dialog, &QDialog::reject);
        dialog.show();
        editing.exec();
    } else
        dialog.exec();
}
} // namespace orbis

#include "live_stream_keys.hpp"
#include "native_forms.hpp"
#include "native_window.hpp"
#include "scene_service.hpp"
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QSlider>
#include <QTimer>
#include <QUuid>
#include <QWizard>
#include <QWizardPage>
#include <algorithm>

namespace orbis {
namespace {
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

// Il form del canale mantiene ingest e chiave separati; TikTok richiede una nuova chiave a ogni
// avvio.
void NativeWindow::editChannel(Json channel) {
    NativeForm dialog(this, t("channelSettings"), channel);
    auto platforms = QList<QPair<QString, Json>>{};
    for (auto name : {"Custom", "Twitch", "YouTube", "Kick", "Facebook", "TikTok"})
        platforms.append({name, Json::string(name)});
    auto platform = dialog.choice("platform", t("platform"), platforms);
    dialog.text("channelName", t("channelName"));
    auto ingest = dialog.text("streamUrl", t("streamUrl"));
    auto key = dialog.text("streamKey", t("streamKey"), true);
    auto reveal = new QCheckBox(t("showStreamKey"), &dialog);
    dialog.form->addRow(reveal);
    connect(reveal, &QCheckBox::toggled, &dialog, [key](bool visible) {
        key->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    });
    auto folder = dialog.text("videoFolder", t("videoFolder"));
    auto browse = new QPushButton(t("sceneMediaChoose"), &dialog);
    dialog.form->addRow(browse);
    connect(browse, &QPushButton::clicked, &dialog, [&, folder] {
        auto path = QFileDialog::getExistingDirectory(&dialog, t("videoFolder"));
        if (!path.isEmpty())
            folder->setText(path);
    });
    dialog.text("description", t("description"));
    dialog.check("isActive", t("active"), true);
    dialog.check("ffmpegSender", t("ffmpegSender"));
    dialog.check("autoCleanupEnabled", t("autoCleanupEnabled"));
    dialog.number("autoCleanupOlderThanMonths", t("autoCleanupOlderThanMonths"), 3, 0, 120);
    dialog.number("autoCleanupIntervalMonths", t("autoCleanupIntervalMonths"), 0, 0, 120);
    auto selectPlatform = [this, platform, ingest, key] {
        const auto name = platform->currentText();
        const QMap<QString, QString> servers = {
            {"Twitch", "rtmp://live.twitch.tv/app"},
            {"YouTube", "rtmps://a.rtmps.youtube.com/live2"},
            {"Kick", "rtmps://fa723fc1b171.global-contribute.live-video.net/app"},
            {"Facebook", "rtmps://rtmp-api.facebook.com:443/rtmp"},
            {"TikTok", "rtmp://push-rtmp-l11-va01.tiktokcdn.com/stage"}};
        if (servers.contains(name))
            ingest->setText(servers.value(name));
        key->setEnabled(name != "TikTok");
        key->setToolTip(name == "TikTok" ? t("keyEachLiveHint") : QString{});
    };
    connect(platform, &QComboBox::currentIndexChanged, &dialog, selectPlatform);
    if (!channel.has("id"))
        selectPlatform();
    else
        key->setEnabled(platform->currentText() != "TikTok");
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto value = dialog.value();
            auto name = platform->currentText().toLower().toStdString();
            value.set("platformStreamName", name);
            if (name == "tiktok")
                value.set("streamKey", standInStreamKey(value.get("channelName").str()));
            validate("channels", value);
            studio.store.save("channels", value, channel.get("id").num());
            dialog.accept();
        });
    });
    dialog.exec();
}

// Le opzioni sono quelle del motore FFmpeg, con limiti coerenti con la validazione del backend.
void NativeWindow::editProfile(Json profile) {
    NativeForm dialog(this, t("videoAndAudioSettings"), profile);
    dialog.text("title", t("title"));
    dialog.check("isActive", t("active"), false);
    dialog.number("videoWidth", t("width"), 1280, 16, 7680);
    dialog.number("videoHeight", t("height"), 720, 16, 7680);
    dialog.number("frameRate", t("frameRate"), 30, 1, 120, 2);
    dialog.number("videoBitrate", t("bitrate") + " (kbps)", 4500, 100, 100000);
    dialog.number("audioBitrate", t("audioBitrate") + " (kbps)", 128, 32, 512);
    dialog.number("gopSize", t("gopSize") + " (s)", 2, 0.1, 10, 1);
    dialog.choice(
        "videoCodecName", t("videoCodecName"),
        {{"libx264", Json::string("libx264")}, {"h264_nvenc", Json::string("h264_nvenc")}});
    dialog.text("preset", t("additionalVideoOptions"));
    dialog.check("lowLatency", t("lowLatency"), true);
    dialog.check("adaptiveBitrate", t("adaptiveBitrate"));
    dialog.choice("encoderQuality", t("encoderQuality"),
                  {{t("encoderQualityAuto"), Json::string("auto")},
                   {t("encoderQualityLight"), Json::string("light")},
                   {t("encoderQualityBalanced"), Json::string("balanced")},
                   {t("encoderQualityHigh"), Json::string("high")}});
    auto options = new QPlainTextEdit(str(profile.get("videoSettingsOptions")), &dialog);
    if (profile.get("videoSettingsOptions").isObject())
        options->setPlainText(QString::fromStdString(profile.get("videoSettingsOptions").dump()));
    options->setObjectName("videoSettingsOptions");
    options->setPlaceholderText("{\"preset\":\"veryfast\",\"tune\":\"zerolatency\"}");
    options->setMaximumHeight(85);
    dialog.form->addRow(t("additionalVideoOptions"), options);
    auto presets = new QComboBox(&dialog);
    presets->setObjectName("profilePresets");
    presets->addItem(t("loadVideoAndAudioSettings"), QString{});
    auto all = studio.store.list("profiles");
    for (size_t i = 0; i < all.size(); ++i)
        if (all.at(i).get("isDefaultConfiguration").flag())
            presets->addItem(str(all.at(i).get("title")), QString::fromStdString(all.at(i).dump()));
    dialog.form->insertRow(0, t("configuration"), presets);
    connect(presets, &QComboBox::currentIndexChanged, &dialog, [&] {
        if (presets->currentData().toString().isEmpty())
            return;
        auto preset = Json::parse(presets->currentData().toString().toStdString());
        for (auto field :
             {"videoWidth", "videoHeight", "frameRate", "videoBitrate", "audioBitrate", "gopSize"})
            if (auto spin = dialog.findChild<QDoubleSpinBox*>(field))
                spin->setValue(preset.get(field).num(spin->value()));
        if (auto low = dialog.findChild<QCheckBox*>("lowLatency"))
            low->setChecked(preset.get("lowLatency").flag(true));
    });
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto value = dialog.value();
            value.set("isDefaultConfiguration", false);
            if (!options->toPlainText().trimmed().isEmpty())
                value.set("videoSettingsOptions",
                          Json::parse(options->toPlainText().toStdString()));
            if (value.get("preset").str().empty())
                value.set("preset", "veryfast");
            validate("profiles", value);
            studio.store.save("profiles", value, profile.get("id").num());
            dialog.accept();
        });
    });
    auto tune = new QPushButton(t("encoderQualityAuto"), &dialog);
    dialog.form->addRow(tune);
    connect(tune, &QPushButton::clicked, &dialog, [&, tune] {
        auto value = dialog.value();
        QPointer<QPushButton> safe(tune);
        runAsync([value] { return tuneEncoder(value); },
                 [safe](Json result) {
                     if (safe)
                         safe->setToolTip(QString::fromStdString(result.dump()));
                 });
    });
    dialog.exec();
}

// La selezione impone due passaggi come Windows e non rende disponibili configurazioni disattivate.
void NativeWindow::startWizard(bool playlist) {
    auto profiles = studio.store.list("profiles"), channels = studio.store.list("channels");
    if (choices(profiles, "title", true).isEmpty()) {
        QMessageBox::information(this, t("warning"), t("wizardNoActiveSettingsMessage"));
        navigate("profiles");
        return;
    }
    if (choices(channels, "channelName", true).isEmpty()) {
        QMessageBox::information(this, t("warning"), t("wizardNoActiveConfigurationsMessage"));
        navigate("channels");
        return;
    }
    QWizard wizard(this);
    wizard.setObjectName("startWizard");
    wizard.setWindowTitle(t(playlist ? "startPlaylistTitle" : "startNewLiveTitle"));
    wizard.setWizardStyle(QWizard::ModernStyle);
    wizard.resize(800, 660);
    wizard.setOption(QWizard::NoBackButtonOnStartPage);
    wizard.setButtonText(QWizard::NextButton, t("next"));
    wizard.setButtonText(QWizard::BackButton, t("back"));
    wizard.setButtonText(QWizard::CancelButton, t("cancel"));
    wizard.setButtonText(QWizard::FinishButton,
                         t(playlist ? "startPlaylistButton" : "composeNext"));
    auto first = new QWizardPage;
    first->setTitle(t("wizardStep1Title"));
    auto firstForm = new QFormLayout(first);
    auto profile = new QListWidget(first);
    profile->setObjectName("wizardProfile");
    profile->setAccessibleName(t("videoAndAudio"));
    profile->setWordWrap(true);
    profile->setTextElideMode(Qt::ElideNone);
    for (auto entry : choices(profiles, "title", true)) {
        const auto value = studio.store.get("profiles", entry.second.num());
        const auto detail = QString("%1 × %2 · %3 fps · %4 kbps · %5")
                                .arg(value.get("videoWidth").num())
                                .arg(value.get("videoHeight").num())
                                .arg(value.get("frameRate").num())
                                .arg(value.get("videoBitrate").num())
                                .arg(str(value.get("videoCodecName")));
        auto item = new QListWidgetItem(entry.first + "\n" + detail, profile);
        item->setSizeHint(QSize(0, 88));
        item->setData(Qt::UserRole, entry.second.num());
    }
    profile->setCurrentRow(0);
    firstForm->addRow(profile);
    wizard.addPage(first);
    auto second = new QWizardPage;
    second->setTitle(t("wizardStep2Title"));
    auto secondForm = new QFormLayout(second);
    auto channel = new QListWidget(second);
    channel->setObjectName("wizardChannel");
    channel->setAccessibleName(t("channel"));
    channel->setWordWrap(true);
    channel->setTextElideMode(Qt::ElideNone);
    for (auto entry : choices(channels, "channelName", true)) {
        const auto value = studio.store.get("channels", entry.second.num());
        auto item = new QListWidgetItem(entry.first + "\n" + str(value.get("platform")), channel);
        item->setSizeHint(QSize(0, 88));
        item->setData(Qt::UserRole, entry.second.num());
    }
    channel->setCurrentRow(0);
    secondForm->addRow(channel);
    auto title = new QLineEdit(second);
    title->setObjectName("wizardTitle");
    title->setPlaceholderText(t("titlePlaceholder"));
    secondForm->addRow(t("title"), title);
    auto key = new QLineEdit(second);
    key->setObjectName("wizardStreamKey");
    key->setEchoMode(QLineEdit::Password);
    secondForm->addRow(t("tiktokKey"), key);
    auto changed = [&] {
        auto c =
            studio.store.get("channels", channel->currentItem()->data(Qt::UserRole).toLongLong());
        const bool needsKey = asksForStreamKey(
            c.get("platform").str(), c.get("platformStreamName").str(), c.get("streamUrl").str());
        key->setVisible(needsKey);
        secondForm->labelForField(key)->setVisible(needsKey);
    };
    connect(channel, &QListWidget::currentRowChanged, &wizard, changed);
    changed();
    wizard.addPage(second);
    auto folder = new QLineEdit(&wizard);
    folder->setObjectName("wizardFolder");
    auto repeat = new QCheckBox(t("nativeLoop"), &wizard);
    repeat->setObjectName("wizardLoop");
    auto countdown = new QDoubleSpinBox(&wizard);
    countdown->setRange(0, 60);
    countdown->setDecimals(0);
    countdown->setValue(3);
    countdown->setObjectName("wizardCountdown");
    auto layoutPicker = new QComboBox(second);
    layoutPicker->addItem(t("blankLayout"), 0);
    auto scenes = studio.store.list("scenes");
    for (size_t i = 0; i < scenes.size(); ++i)
        if (scenes.at(i).get("isLayout").flag())
            layoutPicker->addItem(str(scenes.at(i).get("name")), scenes.at(i).get("id").num());
    if (!playlist)
        secondForm->addRow(t("selectLayout"), layoutPicker);
    if (playlist) {
        auto third = new QWizardPage;
        third->setTitle(t("wizardStep3PlaylistTitle"));
        auto form = new QFormLayout(third);
        auto hint = new QLabel(
            t("playlistOnlyVideos") + " MP4, MKV, MOV, AVI, WebM, M4V, TS, FLV, MPEG, MPG.", third);
        hint->setWordWrap(true);
        form->addRow(hint);
        form->addRow(t("playlistFolder"), folder);
        form->addRow(repeat);
        form->addRow(t("upcomingLive") + " · " + t("sceneDurationSeconds"), countdown);
        auto choose = new QPushButton(t("playlistFolder") + "…", third);
        form->addRow(choose);
        connect(choose, &QPushButton::clicked, &wizard, [&] {
            auto path = QFileDialog::getExistingDirectory(&wizard, t("playlistFolder"));
            if (!path.isEmpty())
                folder->setText(path);
        });
        wizard.addPage(third);
    }
    // La conferma resta aperta finché i dati minimi sono validi, senza perdere la chiave
    // temporanea.
    disconnect(wizard.button(QWizard::FinishButton), nullptr, &wizard, nullptr);
    connect(wizard.button(QWizard::FinishButton), &QAbstractButton::clicked, &wizard, [&] {
        run([&] {
            auto selectedChannel = studio.store.get(
                "channels", channel->currentItem()->data(Qt::UserRole).toLongLong());
            if (asksForStreamKey(selectedChannel.get("platform").str(),
                                 selectedChannel.get("platformStreamName").str(),
                                 selectedChannel.get("streamUrl").str()) &&
                key->text().trimmed().isEmpty())
                throw std::runtime_error("TikTok: chiave richiesta");
            if (playlist && scanVideos(folder->text().toStdString()).empty())
                throw std::runtime_error("Nessun video");
            wizard.accept();
        });
    });
    if (wizard.exec() != QDialog::Accepted)
        return;
    Json request;
    request.set("profileId", profile->currentItem()->data(Qt::UserRole).toDouble());
    request.set("channelId", channel->currentItem()->data(Qt::UserRole).toDouble());
    request.set("title", title->text().isEmpty() ? t("untitled").toStdString()
                                                 : title->text().toStdString());
    if (!key->text().isEmpty())
        request.set("streamKey", key->text().toStdString());
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
                     navigate("preview");
                 });
    } else {
        Json scene;
        if (layoutPicker->currentData().toLongLong()) {
            scene = studio.store.get("scenes", layoutPicker->currentData().toLongLong()).clone();
            scene.set("id", 0.0);
        }
        scene.set("isLayout", false);
        editScene(scene, [this, request](Json scene) mutable {
            request.set("scene", scene);
            runAsync([this, request] { return Json::number(studio.start(request)); },
                     [this](Json id) {
                         watched = id.num();
                         navigate("preview");
                     });
        });
    }
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
    auto channel = studio.store.get("channels", request.get("channelId").num());
    if (asksForStreamKey(channel.get("platform").str(), channel.get("platformStreamName").str(),
                         channel.get("streamUrl").str())) {
        NativeForm dialog(this, t("tiktokKey"));
        dialog.text("streamKey", t("tiktokKey"), true);
        connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        if (dialog.exec() != QDialog::Accepted)
            return;
        request.set("streamKey", dialog.value().get("streamKey"));
    }
    runAsync([this, request] { return Json::number(studio.start(request)); },
             [this](Json id) {
                 watched = id.num();
                 navigate("preview");
             });
}

// Aggiorna il profilo attivo e quello da riusare nei riavvii, senza alterare le altre richieste
// storiche.
void NativeWindow::linkProfile(Json history) {
    NativeForm dialog(this, t("connectVideoSetting"));
    dialog.choice("profileId", t("videoAndAudio"),
                  choices(studio.store.list("profiles"), "title", true));
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto value = dialog.value();
            auto profile = studio.store.get("profiles", value.get("profileId").num());
            auto id = history.get("id").num();
            if (auto session = studio.session(id); session && !session->done())
                session->command("parameters", profile);
            auto stored = studio.store.get("history", id);
            auto request = stored.get("request").clone();
            request.set("profileId", value.get("profileId"));
            request.set("profile", profile);
            stored.set("request", request);
            studio.store.save("history", stored, id);
            dialog.accept();
        });
    });
    dialog.exec();
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
    const bool scene = history.get("request").has("scene");
    dialog.setWindowTitle(t(scene ? "sceneDetails" : "playlistDetails"));
    auto layout = new QVBoxLayout(&dialog);
    auto list = new QListWidget(&dialog);
    auto videos = history.get("request").get("files");
    if (!videos.isArray())
        videos = history.get("videos");
    if (scene) {
        const auto items = history.get("request").get("scene").get("items");
        for (size_t i = 0; i < items.size(); ++i)
            list->addItem(str(items.at(i).get("label")) + " · " +
                          str(items.at(i).get("sourceKind")));
    }
    for (size_t i = 0; !scene && i < videos.size(); ++i)
        list->addItem(videos.at(i).isObject() ? str(videos.at(i).get("path")) : str(videos.at(i)));
    layout->addWidget(list);
    auto play = new QPushButton(t("playlistStartFrom"), &dialog);
    play->setVisible(!scene);
    play->setEnabled(false);
    connect(list, &QListWidget::currentRowChanged, &dialog,
            [play](int row) { play->setEnabled(row >= 0); });
    layout->addWidget(play);
    connect(play, &QPushButton::clicked, &dialog, [&] {
        if (list->currentRow() >= 0) {
            auto index = list->currentRow();
            dialog.accept();
            replay(history, false, index);
        }
    });
    auto details = new QPushButton(t("seeDetails"), &dialog);
    layout->addWidget(details);
    connect(details, &QPushButton::clicked, &dialog, [&] { showSnapshot(history); });
    dialog.resize(720, 500);
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
        volume->setValue(items.at(i).get("volume").num(1) * 100);
        auto mute = new QCheckBox(t("volumeMute"), row);
        mute->setChecked(!items.at(i).get("audioEnabled").flag(true));
        horizontal->addWidget(volume);
        horizontal->addWidget(mute);
        form->addRow(str(items.at(i).get("sourceTarget")), row);
        auto apply = [this, id, i, volume, mute] {
            run([&] {
                Json command;
                command.set("index", double(i));
                command.set("volume", volume->value() / 100.0);
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
    armedDeck = id;
    for (auto shortcut : deckShortcuts)
        delete shortcut;
    deckShortcuts.clear();
    QDialog dialog(this);
    dialog.setObjectName("sceneDeckDialog");
    dialog.setWindowTitle(t("sceneDeck"));
    dialog.resize(760, 540);
    auto layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    auto title = new QLabel(t("sceneDeck"), &dialog);
    title->setObjectName("heroTitle");
    layout->addWidget(title);
    auto hint = new QLabel(t("sceneDeckHint"), &dialog);
    hint->setWordWrap(true);
    hint->setObjectName("muted");
    layout->addWidget(hint);
    auto scroll = new QScrollArea(&dialog);
    scroll->setWidgetResizable(true);
    auto deck = new QWidget;
    auto grid = new QGridLayout(deck);
    scroll->setWidget(deck);
    layout->addWidget(scroll, 1);
    auto rows = studio.store.list("buttons");
    if (rows.size() == 0)
        grid->addWidget(new QLabel(t("sceneEmpty"), deck), 0, 0);
    for (size_t i = 0; i < rows.size(); ++i) {
        auto button = rows.at(i);
        auto push =
            new QPushButton(str(button.get("label")) + "\n" + str(button.get("hotkey")), deck);
        push->setMinimumHeight(84);
        push->setCursor(Qt::PointingHandCursor);
        grid->addWidget(push, int(i) / 3, int(i) % 3);
        auto activate = [this, id, button] {
            if (confirm(button.get("displayMode").str() == "in_scene" ? "sceneConfirmInScene"
                                                                      : "sceneConfirm",
                        str(button.get("label"))))
                run([&] {
                    Json data;
                    data.set("id", button.get("id"));
                    liveCommand(id, "button", data);
                });
        };
        connect(push, &QPushButton::clicked, &dialog, activate);
        auto key = str(button.get("hotkey"));
        key.replace("Key", "");
        key.replace("Digit", "");
        if (!key.isEmpty()) {
            auto shortcut = new QShortcut(QKeySequence(key), &dialog);
            shortcut->setContext(Qt::WidgetWithChildrenShortcut);
            connect(shortcut, &QShortcut::activated, &dialog, activate);
            auto armed = new QShortcut(QKeySequence(key), this);
            armed->setContext(Qt::WindowShortcut);
            connect(armed, &QShortcut::activated, this, [this, id, activate] {
                if (armedDeck == id && (page == "live" || page == "preview"))
                    activate();
            });
            deckShortcuts.append(armed);
        }
    }
    auto resume = new QPushButton(t("sceneResume"), &dialog);
    resume->setProperty("primary", true);
    layout->addWidget(resume);
    connect(resume, &QPushButton::clicked, &dialog,
            [this, id] { run([&] { liveCommand(id, "resume"); }); });
    auto disarm = new QPushButton(t("sceneKeysOff"), &dialog);
    layout->addWidget(disarm);
    connect(disarm, &QPushButton::clicked, &dialog, [this] { armedDeck = 0; });
    auto config = new QPushButton(t("sceneAdd"), &dialog);
    layout->addWidget(config);
    connect(config, &QPushButton::clicked, &dialog, [&] {
        editButton();
        dialog.accept();
    });
    auto keysHint = new QLabel(t("sceneKeysHint"), &dialog);
    keysHint->setWordWrap(true);
    keysHint->setObjectName("muted");
    layout->addWidget(keysHint);
    auto close = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(close, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(close);
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
void NativeWindow::editButton(Json button) {
    NativeForm dialog(this, t(button.has("id") ? "sceneEdit" : "sceneAdd"), button);
    dialog.text("label", t("sceneButtonName"));
    dialog.number("position", t("scenePlacement"), 0, 0, 5);
    auto key = dialog.text("hotkey", t("sceneKey"));
    key->setPlaceholderText("Ctrl+KeyB / F1");
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
    dialog.choice("displayMode", t("sceneDisplayMode"),
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
    dialog.choice("placement", t("scenePlacement"), placements);
    dialog.number("durationSeconds", t("sceneDurationSeconds"), 0, 0, 86400);
    for (auto field : {"x", "y", "width", "height"})
        dialog.number(field,
                      field == std::string("width")    ? t("width")
                      : field == std::string("height") ? t("height")
                                                       : QString(field),
                      field == std::string("width")    ? 320
                      : field == std::string("height") ? 180
                                                       : 0,
                      0, 7680);
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto value = dialog.value();
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
    dialog.exec();
}
} // namespace orbis

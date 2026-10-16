#include "native_theme.hpp"
#include "native_window.hpp"
#include "scene_service.hpp"
#include <QAudioOutput>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlayer>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QSlider>
#include <QSplitter>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QVBoxLayout>
#include <QVideoWidget>

namespace orbis {
// Player Qt Multimedia incorporato: audio, video e seek sono controlli dell'app, senza browser.
void NativeWindow::playFile(const QString& path) {
    // Qt 6.4 offre un backend FFmpeg: riusa i codec già richiesti dal motore, evitando decoder
    // GStreamer mancanti. L’override dell’utente viene rispettato; Qt meno recenti restano sul
    // proprio backend di sistema.
    if (!qEnvironmentVariableIsSet("QT_MEDIA_BACKEND")) {
        for (const auto& plugins : QCoreApplication::libraryPaths()) {
            if (QFileInfo::exists(plugins + "/multimedia/libffmpegmediaplugin.so")) {
                qputenv("QT_MEDIA_BACKEND", "ffmpeg");
                break;
            }
        }
    }
    QDialog dialog(this);
    dialog.setWindowTitle(t("preview"));
    dialog.resize(880, 600);
    auto layout = new QVBoxLayout(&dialog);
    auto video = new QVideoWidget(&dialog);
    layout->addWidget(video, 1);
    auto player = new QMediaPlayer(&dialog);
    auto audio = new QAudioOutput(&dialog);
    player->setVideoOutput(video);
    player->setAudioOutput(audio);
    player->setSource(QUrl::fromLocalFile(path));
    auto seek = new QSlider(Qt::Horizontal, &dialog);
    layout->addWidget(seek);
    connect(player, &QMediaPlayer::durationChanged, &dialog,
            [seek](qint64 duration) { seek->setMaximum(duration); });
    connect(player, &QMediaPlayer::positionChanged, &dialog, [seek](qint64 position) {
        if (!seek->isSliderDown())
            seek->setValue(position);
    });
    connect(seek, &QSlider::sliderReleased, &dialog,
            [player, seek] { player->setPosition(seek->value()); });
    auto play = new QPushButton(t("preview"), &dialog);
    layout->addWidget(play);
    connect(play, &QPushButton::clicked, &dialog, [player] {
        if (player->playbackState() == QMediaPlayer::PlayingState)
            player->pause();
        else
            player->play();
    });
    auto proxy = new QPushButton(t("retry"), &dialog);
    layout->addWidget(proxy);
    auto conversion = new QTimer(&dialog);
    conversion->setInterval(500);
    connect(proxy, &QPushButton::clicked, &dialog, [this, path, conversion] {
        run([&] {
            proxies.prepare(path.toStdString());
            conversion->start();
        });
    });
    connect(conversion, &QTimer::timeout, &dialog, [this, path, conversion, player] {
        run([&] {
            auto state = proxies.prepare(path.toStdString());
            if (state.get("status").str() == "READY") {
                conversion->stop();
                player->setSource(QUrl::fromLocalFile(QString::fromStdString(
                    (studio.store.root / "proxies" / state.get("name").str()).string())));
                player->play();
            } else if (state.get("status").str() == "ERROR") {
                conversion->stop();
                throw std::runtime_error("Proxy non disponibile");
            }
        });
    });
    auto error = new QLabel(&dialog);
    layout->addWidget(error);
    connect(player, &QMediaPlayer::errorOccurred, &dialog,
            [this, error](QMediaPlayer::Error, const QString&) {
                error->setText(t("notPlayableHere"));
            });
    player->play();
    dialog.exec();
    player->stop();
}

// Librerie native con copie private; gli overlay ancora referenziati sono protetti dalla
// cancellazione.
void NativeWindow::buildMedia(const QString& kind) {
    const auto directory = studio.store.root / kind.toStdString();
    fs::create_directories(directory);
    addAction(kind == "overlays" ? "uploadOverlay" : "sceneMediaChoose", [this, directory, kind] {
        auto files = QFileDialog::getOpenFileNames(
            this, t("sceneMedia"), {},
            "Media (*.png *.jpg *.jpeg *.gif *.webp *.apng *.webm *.mov *.mp4 *.mkv)");
        if (files.isEmpty())
            return;
        for (const auto& file : files) {
            auto target =
                directory / (QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString() +
                             fs::path(file.toStdString()).extension().string());
            fs::copy_file(file.toStdString(), target);
            fs::permissions(target, fs::perms::owner_read | fs::perms::owner_write);
        }
        navigate(kind);
    });
    auto list = makeTable({t("sceneMedia"), t("path"), t("settings")});
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (!entry.is_regular_file())
            continue;
        int row = list->rowCount();
        list->insertRow(row);
        list->setItem(
            row, 0, new QTableWidgetItem(QString::fromStdString(entry.path().filename().string())));
        list->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(entry.path().string())));
        auto controls = new QWidget(list);
        auto actions = new QHBoxLayout(controls);
        auto preview = new QPushButton(t("preview"), controls);
        actions->addWidget(preview);
        connect(preview, &QPushButton::clicked, this, [this, path = entry.path()] {
            run([&] { playFile(QString::fromStdString(path.string())); });
        });
        auto remove = new QPushButton(t("delete"), controls);
        actions->addWidget(remove);
        connect(remove, &QPushButton::clicked, this, [this, path = entry.path(), kind] {
            run([&] {
                if (!confirm("overlayDeleteConfirm"))
                    return;
                if (kind == "overlays" && overlayInUse(studio.store, path.string()))
                    throw std::runtime_error("Overlay utilizzato");
                if (kind == "scene-media") {
                    auto buttons = studio.store.list("buttons");
                    for (size_t i = 0; i < buttons.size(); ++i)
                        if (buttons.at(i).get("mediaName").str() == path.filename().string())
                            throw std::runtime_error("Media utilizzato");
                }
                fs::remove(path);
                navigate(kind);
            });
        });
        list->setCellWidget(row, 2, controls);
    }
    // Questo elenco viene popolato dai file: i campionamenti dello store non devono sostituirlo.
    table = nullptr;
}

// I sei pulsanti usano record persistenti e restano configurabili anche senza una diretta attiva.
void NativeWindow::buildButtons() {
    addAction("sceneAdd", [this] { editButton(); });
    addAction("sceneMedia", [this] { navigate("scene-media"); });
    table = makeTable({t("sceneButtonName"), t("sceneMedia"), t("sceneKey"), t("settings")});
}

// La libreria conserva percorsi e ordine; importare una cartella non copia o modifica gli
// originali.
void NativeWindow::buildPlaylist() {
    addAction("startPlaylistButton", [this] { startWizard(true); });
    addAction("addVideoFile", [this] { editVideo(); });
    addAction("videoFolder", [this] {
        auto folder = QFileDialog::getExistingDirectory(this, t("videoFolder"));
        if (folder.isEmpty())
            return;
        runAsync([this, folder] {
            auto files = scanVideos(folder.toStdString());
            auto rows = studio.store.list("videos");
            auto channels = studio.store.list("channels");
            size_t order = rows.size();
            for (const auto& file : files) {
                bool present = false;
                for (size_t i = 0; i < rows.size(); ++i)
                    if (rows.at(i).get("videoPath").str() == file)
                        present = true;
                if (present)
                    continue;
                Json video;
                video.set("name", fs::path(file).filename().string());
                video.set("videoPath", file);
                video.set("enabled", true);
                video.set("order", double(order++));
                if (channels.size())
                    video.set("channelId", channels.at(0).get("id"));
                studio.store.save("videos", video);
            }
            return Json::number(files.size());
        });
    });
    addAction("unLockVideo", [this] {
        auto videos = studio.store.list("videos");
        for (size_t i = 0; i < videos.size(); ++i) {
            auto video = videos.at(i).clone();
            video.set("isLocked", false);
            studio.store.save("videos", video, video.get("id").num());
        }
        sample();
    });
    table = makeTable({t("title"), t("path"), t("active"), t("settings")});
}

// Il manuale usa testo e immagini Qt, con gli stessi capitoli tradotti del progetto Windows.
void NativeWindow::buildManual() {
    auto document = Json::parse(
        readFile(resources / "ui" / ("manual." + translations.language().toStdString() + ".json")));
    auto sections = document.get("sections");
    auto split = new QSplitter(body);
    auto toc = new QListWidget(split);
    toc->setObjectName("manualContents");
    auto text = new QTextBrowser(split);
    text->setObjectName("manualText");
    text->setReadOnly(true);
    split->addWidget(toc);
    split->addWidget(text);
    split->setStretchFactor(1, 3);
    split->setMinimumHeight(500);
    bodyLayout->addWidget(split, 1);
    for (size_t i = 0; i < sections.size(); ++i) {
        const auto id = sections.at(i).get("id").str();
        const QString icon = id == "dashboard"     ? "home"
                             : id == "history"     ? "history"
                             : id == "taskManager" ? "activity"
                             : id == "sceneDeck"   ? "deck"
                             : id == "preview"     ? "screen"
                             : id == "layouts"     ? "layouts"
                             : id == "settings"    ? "settings"
                                                   : "video";
        toc->addItem(new QListWidgetItem(
            studioIcon(icon), QString::number(i + 1) + "  " +
                                  QString::fromStdString(sections.at(i).get("title").str())));
    }
    auto content = [this, sections](int index) {
        if (index < 0)
            return QString{};
        auto section = sections.at(index);
        auto escape = [](Json value) {
            return QString::fromStdString(value.str()).toHtmlEscaped();
        };
        QString html = "<a name='section-" + QString::number(index) + "'></a><h1>" +
                       QString::number(index + 1) + ". " + escape(section.get("title")) +
                       "</h1><p>" + escape(section.get("body")) + "</p><ol>";
        auto steps = section.get("steps");
        for (size_t i = 0; i < steps.size(); ++i)
            html += "<li>" + escape(steps.at(i)) + "</li>";
        html += "</ol>";
        auto image = resources / "manual" / translations.language().toStdString() /
                     (section.get("figure").str() + ".webp");
        if (fs::exists(image))
            html += "<p><img width='720' src='" +
                    QUrl::fromLocalFile(QString::fromStdString(image.string())).toString() +
                    "'></p>";
        html += "<p>" + escape(section.get("callout")) + "</p>";
        return html;
    };
    QString guide;
    for (size_t i = 0; i < sections.size(); ++i)
        guide += content(i);
    text->setHtml(guide);
    text->setOpenExternalLinks(true);
    connect(toc, &QListWidget::currentRowChanged, this,
            [text](int row) { text->scrollToAnchor("section-" + QString::number(row)); });
    toc->setCurrentRow(0);
    auto hero = body->findChild<QFrame*>("pageHeading");
    auto row = qobject_cast<QHBoxLayout*>(hero->layout());
    auto mark = new QLabel(hero);
    mark->setPixmap(QPixmap(QString::fromStdString((resources / "app-icon.png").string()))
                        .scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    row->insertWidget(0, mark);
    body->findChild<QLabel*>("pageTitle")->setText(t("manualTitle"));
    auto actions = new QWidget(hero);
    new QHBoxLayout(actions);
    row->addWidget(actions);
    // QTextDocument stampa il documento, non un sito: la stessa impaginazione produce anche un PDF.
    addAction(
        "manualDownloadPdf",
        [this, sections, content] {
            auto path = QFileDialog::getSaveFileName(this, t("manualDownloadPdf"),
                                                     "OrbisStream.pdf", "PDF (*.pdf)");
            if (path.isEmpty())
                return;
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(path);
            QTextDocument doc;
            QString html;
            for (size_t i = 0; i < sections.size(); ++i)
                html += content(i);
            doc.setHtml(html);
            doc.print(&printer);
        },
        actions, "download");
    addAction(
        "manualPrint",
        [this, sections, content] {
            QPrinter printer(QPrinter::HighResolution);
            QPrintDialog dialog(&printer, this);
            if (dialog.exec() != QDialog::Accepted)
                return;
            QTextDocument doc;
            QString html;
            for (size_t i = 0; i < sections.size(); ++i)
                html += content(i);
            doc.setHtml(html);
            doc.print(&printer);
        },
        actions, "print");
}
} // namespace orbis

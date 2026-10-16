#include "native_forms.hpp"
#include "native_theme.hpp"
#include "native_window.hpp"
#include "scene_service.hpp"
#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMimeData>
#include <QPainter>
#include <QSplitter>
#include <QStyle>
#include <QTimer>
#include <QUuid>
#include <algorithm>

namespace orbis {
namespace {
// The catalog supports both click-to-add and the same drag payload used by the canvas.
class SourceButton : public QPushButton {
    Json source;
    QPoint pressed;

  public:
    SourceButton(QString label, Json source, QWidget* parent)
        : QPushButton(label, parent), source(source) {}

  protected:
    void mousePressEvent(QMouseEvent* event) override {
        pressed = event->pos();
        QPushButton::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (!(event->buttons() & Qt::LeftButton) ||
            (event->pos() - pressed).manhattanLength() < QApplication::startDragDistance())
            return;
        auto drag = new QDrag(this);
        auto mime = new QMimeData;
        mime->setData("application/x-orbis-source", QByteArray::fromStdString(source.dump()));
        drag->setMimeData(mime);
        drag->exec(Qt::CopyAction);
        setDown(false);
    }
};

class OverlayList : public QListWidget {
  public:
    using QListWidget::QListWidget;

  protected:
    QMimeData* mimeData(const QList<QListWidgetItem*>& items) const override {
        auto mime = new QMimeData;
        QList<QUrl> urls;
        for (auto item : items)
            urls.append(QUrl::fromLocalFile(item->data(Qt::UserRole).toString()));
        mime->setUrls(urls);
        return mime;
    }
};

// Un elemento grafico conserva il JSON della sorgente: spostamento e ridimensionamento aggiornano
// la geometria salvata.
class SourceTile : public QGraphicsRectItem {
    bool resizing = false;
    double aspect = 1;

  public:
    Json source;

    SourceTile(Json item)
        : QGraphicsRectItem(0, 0, item.get("width").num(1280), item.get("height").num(720)),
          source(item.clone()) {
        setPos(item.get("x").num(), item.get("y").num());
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
        setBrush(QColor(40, 110, 180, 100));
        setPen(QPen(QColor(90, 170, 240), 3));
        auto title = new QGraphicsSimpleTextItem(
            QString::fromStdString(item.get("label").str(item.get("sourceKind").str())), this);
        title->setBrush(Qt::white);
        title->setPos(8, 8);
        aspect = rect().width() / rect().height();
    }

    Json value() const {
        auto result = source.clone();
        result.set("x", pos().x());
        result.set("y", pos().y());
        result.set("width", rect().width());
        result.set("height", rect().height());
        return result;
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
        if (change == ItemPositionChange && scene()) {
            auto p = value.toPointF();
            auto bounds = scene()->sceneRect();
            p.setX(std::clamp(p.x(), 0.0, std::max(0.0, bounds.width() - rect().width())));
            p.setY(std::clamp(p.y(), 0.0, std::max(0.0, bounds.height() - rect().height())));
            return p;
        }
        return QGraphicsRectItem::itemChange(change, value);
    }

    // L'angolo in basso a destra ridimensiona; Shift libera il rapporto come nel composer Windows.
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        resizing =
            event->pos().x() > rect().width() - 25 && event->pos().y() > rect().height() - 25;
        if (resizing) {
            setSelected(true);
            event->accept();
        } else
            QGraphicsRectItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override {
        if (!resizing) {
            QGraphicsRectItem::mouseMoveEvent(event);
            return;
        }
        auto bounds = scene()->sceneRect();
        double w = std::clamp(event->pos().x(), 16.0, std::max(16.0, bounds.width() - pos().x())),
               h = std::clamp(event->pos().y(), 16.0, std::max(16.0, bounds.height() - pos().y()));
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            h = w / aspect;
            if (h > bounds.height() - pos().y()) {
                h = bounds.height() - pos().y();
                w = h * aspect;
            }
        }
        setRect(0, 0, w, h);
        event->accept();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override {
        resizing = false;
        QGraphicsRectItem::mouseReleaseEvent(event);
    }
};

// QGraphicsView usa eventi Qt reali, inclusi file dal file manager e tasti per le sorgenti
// selezionate.
class CanvasView : public QGraphicsView {
  public:
    std::function<void(QString)> fileDropped;
    std::function<void(Json)> sourceDropped;
    QString emptyHint;

    explicit CanvasView(QGraphicsScene* scene, QWidget* parent) : QGraphicsView(scene, parent) {
        setObjectName("sceneCanvas");
        setAcceptDrops(true);
        setRenderHint(QPainter::Antialiasing);
        setMinimumSize(420, 260);
        setBackgroundBrush(Qt::black);
    }

    void drawBackground(QPainter* painter, const QRectF& area) override {
        QGraphicsView::drawBackground(painter, area);
        painter->setPen(QPen(QColor("#151515"), 0));
        const auto bounds = scene()->sceneRect();
        for (int i = 1; i < 3; ++i) {
            painter->drawLine(QPointF(bounds.width() * i / 3, 0),
                              QPointF(bounds.width() * i / 3, bounds.height()));
            painter->drawLine(QPointF(0, bounds.height() * i / 3),
                              QPointF(bounds.width(), bounds.height() * i / 3));
        }
    }

    // L'aiuto sul canvas vuoto non diventa una sorgente né entra nei dati salvati.
    void drawForeground(QPainter* painter, const QRectF&) override {
        if (!scene()->items().isEmpty())
            return;
        painter->save();
        painter->resetTransform();
        painter->setPen(QColor("#b9b9c0"));
        painter->drawText(viewport()->rect().adjusted(36, 24, -36, -24),
                          Qt::AlignCenter | Qt::TextWordWrap, emptyHint);
        painter->restore();
    }

    void resizeEvent(QResizeEvent* event) override {
        QGraphicsView::resizeEvent(event);
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    }

    void keyPressEvent(QKeyEvent* event) override {
        auto selected = scene()->selectedItems();
        if (selected.isEmpty()) {
            QGraphicsView::keyPressEvent(event);
            return;
        }
        auto item = selected.first();
        const double step = event->modifiers() & Qt::ShiftModifier ? 10 : 1;
        switch (event->key()) {
        case Qt::Key_Delete:
            delete item;
            break;
        case Qt::Key_Left:
            item->moveBy(-step, 0);
            break;
        case Qt::Key_Right:
            item->moveBy(step, 0);
            break;
        case Qt::Key_Up:
            item->moveBy(0, -step);
            break;
        case Qt::Key_Down:
            item->moveBy(0, step);
            break;
        default:
            QGraphicsView::keyPressEvent(event);
            return;
        }
        event->accept();
    }

    void dragEnterEvent(QDragEnterEvent* event) override {
        if (event->mimeData()->hasUrls() ||
            event->mimeData()->hasFormat("application/x-orbis-source"))
            event->acceptProposedAction();
        else
            QGraphicsView::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (event->mimeData()->hasUrls() ||
            event->mimeData()->hasFormat("application/x-orbis-source"))
            event->acceptProposedAction();
        else
            QGraphicsView::dragMoveEvent(event);
    }

    void dropEvent(QDropEvent* event) override {
        if (event->mimeData()->hasFormat("application/x-orbis-source") && sourceDropped) {
            sourceDropped(
                Json::parse(event->mimeData()->data("application/x-orbis-source").toStdString()));
            event->acceptProposedAction();
            return;
        }
        for (const auto& url : event->mimeData()->urls())
            if (url.isLocalFile() && fileDropped)
                fileDropped(url.toLocalFile());
        event->acceptProposedAction();
    }
};
} // namespace

// L'editor nativo lavora su una copia: annullare non modifica layout o scene già utilizzati dalle
// dirette.
bool NativeWindow::editScene(Json scene, std::function<void(Json)> saved) {
    const bool isLayout = scene.get("isLayout").flag();
    NativeForm dialog(this, t(isLayout ? "saveLayout" : "composeTitle"), scene);
    dialog.setObjectName("sceneEditor");
    dialog.resize(
        (saved || isLayout) ? std::max(width(), dialog.screen()->availableGeometry().width()) - 28
                            : 1160,
        (saved || isLayout) ? dialog.screen()->availableGeometry().height() - 28
                            : qMin(900, dialog.screen()->availableGeometry().height() - 60));
    if (saved || isLayout) {
        dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        auto title = dialog.findChild<QLabel*>("heroTitle");
        dialog.layout->removeWidget(title);
        auto header = new QFrame(&dialog);
        header->setObjectName("composerHeading");
        auto row = new QHBoxLayout(header);
        auto titles = new QVBoxLayout;
        title->setObjectName("dialogTitle");
        if (isLayout)
            title->setText(t("layouts"));
        titles->addWidget(title);
        auto description =
            new QLabel(t(isLayout ? "layoutsDescription" : "composeSubtitle"), header);
        description->setObjectName("muted");
        description->setWordWrap(true);
        titles->addWidget(description);
        row->addLayout(titles, 1);
        auto back = new QPushButton(t("back"), header);
        back->setAutoDefault(false);
        back->setObjectName("composerBack");
        back->setIcon(studioIcon("back"));
        connect(back, &QPushButton::clicked, &dialog, [&] {
            dialog.setProperty("backRequested", true);
            dialog.reject();
        });
        row->addWidget(back);
        auto close = new QPushButton(header);
        close->setAutoDefault(false);
        close->setObjectName("composerClose");
        close->setIcon(studioIcon("close"));
        close->setAccessibleName(t("close"));
        connect(close, &QPushButton::clicked, &dialog, &QDialog::reject);
        row->addWidget(close);
        dialog.layout->insertWidget(0, header);
        dialog.layout->setContentsMargins(15, 0, 15, 15);
    }
    // Un pulsante personalizzato conserva il verbo anche quando Qt ritraduce i pulsanti standard.
    auto standardSave = dialog.buttons->button(QDialogButtonBox::Save);
    dialog.buttons->removeButton(standardSave);
    delete standardSave;
    auto commit = dialog.buttons->addButton(t(saved      ? "startSceneLive"
                                              : isLayout ? "saveLayout"
                                                         : "save"),
                                            QDialogButtonBox::AcceptRole);
    commit->setObjectName("commitScene");
    commit->setIcon(studioIcon(saved ? "play" : "save"));
    commit->setDefault(true);
    commit->setProperty("primary", true);
    commit->style()->unpolish(commit);
    commit->style()->polish(commit);
    auto formScroll = dialog.findChild<QScrollArea*>();
    dialog.layout->removeWidget(formScroll);
    auto workspace = new QSplitter(Qt::Horizontal, &dialog);
    workspace->setChildrenCollapsible(false);
    dialog.layout->insertWidget(1, workspace, 1);
    auto sourceScroll = new QScrollArea(workspace);
    sourceScroll->setWidgetResizable(true);
    auto sourcePanel = new QWidget;
    sourcePanel->setMinimumWidth(220);
    auto sourceLayout = new QVBoxLayout(sourcePanel);
    sourceLayout->setContentsMargins(0, 0, 8, 0);
    sourceLayout->setSpacing(12);
    sourceScroll->setWidget(sourcePanel);
    auto mainPanel = new QWidget(workspace);
    auto mainLayout = new QVBoxLayout(mainPanel);
    mainLayout->setContentsMargins(8, 0, 0, 0);
    formScroll->setMinimumHeight(205);
    formScroll->setMaximumHeight(220);
    mainLayout->addWidget(formScroll);
    workspace->setStretchFactor(0, 0);
    workspace->setStretchFactor(1, 1);
    auto layerScroll = new QScrollArea(workspace);
    layerScroll->setWidgetResizable(true);
    auto layerPanel = new QWidget(layerScroll);
    layerPanel->setMinimumWidth(220);
    auto layerColumn = new QVBoxLayout(layerPanel);
    layerScroll->setWidget(layerPanel);
    workspace->setSizes({230, dialog.width() - 490, 240});
    auto availableCard = new QFrame(sourcePanel);
    availableCard->setObjectName("card");
    auto availableColumn = new QVBoxLayout(availableCard);
    availableColumn->setContentsMargins(14, 14, 14, 14);
    sourceLayout->addWidget(availableCard);
    auto overlaysCard = new QFrame(sourcePanel);
    overlaysCard->setObjectName("card");
    auto overlaysColumn = new QVBoxLayout(overlaysCard);
    overlaysColumn->setContentsMargins(14, 14, 14, 14);
    sourceLayout->addWidget(overlaysCard);
    auto sourceTitle = new QLabel(t(isLayout ? "slots" : "availableSources"), sourcePanel);
    sourceTitle->setObjectName("cardTitle");
    availableColumn->addWidget(sourceTitle);
    auto explanation = new QLabel(t(isLayout ? "slotsHint" : "canvasComposeHint"), sourcePanel);
    explanation->setObjectName("muted");
    explanation->setWordWrap(true);
    if (isLayout)
        availableColumn->addWidget(explanation);
    else
        explanation->hide();
    auto name = dialog.text("name", t(isLayout ? "layoutName" : "sceneName"));
    if (name->text().trimmed().isEmpty())
        name->setText(t(isLayout ? "newLayout" : "newScene"));
    dialog.text("description", t("description"));
    auto width = dialog.number("width", t("width"), 1920, 16, 7680),
         height = dialog.number("height", t("height"), 1080, 16, 7680);
    auto bar = new QFrame(mainPanel);
    bar->setObjectName("card");
    auto barRow = new QHBoxLayout(bar);
    auto layoutPicker = new QComboBox(bar);
    layoutPicker->setObjectName("composerLayouts");
    layoutPicker->addItem(t(isLayout ? "newLayout" : "noLayoutSelected"), 0);
    auto layouts = studio.store.list("scenes");
    for (size_t i = 0; i < layouts.size(); ++i)
        if (layouts.at(i).get("isLayout").flag())
            layoutPicker->addItem(QString::fromStdString(layouts.at(i).get("name").str()),
                                  qlonglong(layouts.at(i).get("id").num()));
    auto resolution = new QComboBox(bar);
    resolution->setObjectName("composerResolution");
    for (auto size : {"1280x720", "1920x1080", "2560x1440", "3840x2160", "720x1280", "1080x1920"})
        resolution->addItem(size);
    resolution->setCurrentText(QString::number(int(width->value())) + "x" +
                               QString::number(int(height->value())));
    for (auto entry :
         QList<QPair<QString, QWidget*>>{{t("layout"), layoutPicker},
                                         {t(isLayout ? "layoutName" : "sceneName"), name},
                                         {t("resolution"), resolution}}) {
        auto column = new QVBoxLayout;
        column->addWidget(new QLabel(entry.first, bar));
        column->addWidget(entry.second);
        barRow->addLayout(column, entry.second == name ? 1 : 0);
    }
    mainLayout->removeWidget(formScroll);
    formScroll->hide();
    mainLayout->insertWidget(0, bar);
    if (saved || isLayout) {
        dialog.buttons->removeButton(commit);
        barRow->addWidget(commit, 0, Qt::AlignBottom);
        dialog.buttons->hide();
        connect(commit, &QPushButton::clicked, dialog.buttons, &QDialogButtonBox::accepted);
    }
    connect(resolution, &QComboBox::currentTextChanged, &dialog, [&](const QString& value) {
        auto parts = value.split('x');
        width->setValue(parts[0].toInt());
        height->setValue(parts[1].toInt());
    });
    auto graphics = new QGraphicsScene(&dialog);
    graphics->setSceneRect(0, 0, width->value(), height->value());
    auto view = new CanvasView(graphics, &dialog);
    view->emptyHint =
        t("canvasEmpty") + "\n\n" + t(isLayout ? "layoutEmptyHint" : "canvasComposeHint");
    mainLayout->addWidget(view, 1);
    // I livelli sono l'ordine dei sourceItems, che FFmpeg compone dal fondo verso il primo piano.
    auto addSource = [&](Json item) {
        if (graphics->items().size() >= 64)
            throw std::runtime_error("Massimo 32 sorgenti");
        if (!item.has("width")) {
            item.set("width", graphics->items().isEmpty() ? width->value() : width->value() / 3);
            item.set("height", graphics->items().isEmpty() ? height->value() : height->value() / 3);
        }
        auto tile = new SourceTile(item);
        tile->setZValue(graphics->items().size());
        graphics->addItem(tile);
        graphics->clearSelection();
        tile->setSelected(true);
    };
    auto items = scene.get("items");
    for (size_t i = 0; i < items.size(); ++i)
        addSource(items.at(i));
    auto layerActions = new QWidget(layerPanel);
    auto layerRow = new QGridLayout(layerActions);
    layerRow->setContentsMargins(0, 0, 0, 0);
    auto action = [&](const char* key, std::function<void()> fn) {
        const QString name(key);
        const bool layerAction = name == "bringForward" || name == "sendBackward" ||
                                 name == "fillCanvas" || name == "removeSource";
        auto button = new QPushButton(t(key), layerAction ? layerActions : sourcePanel);
        button->setAutoDefault(false);
        button->setObjectName(key);
        button->setIcon(studioIcon(name == "uploadOverlay"    ? "upload"
                                   : name == "refreshSources" ? "restart"
                                                              : "plus"));
        button->setCursor(Qt::PointingHandCursor);
        if (layerAction) {
            button->setToolTip(t(key));
            button->setAccessibleName(t(key));
            button->setText({});
            button->setIcon(studioIcon(name == "bringForward"   ? "next"
                                       : name == "sendBackward" ? "back"
                                       : name == "removeSource" ? "delete"
                                                                : "screen"));
            button->setFixedSize(36, 36);
            const int index = layerRow->count();
            layerRow->addWidget(button, index / 4, index % 4);
        } else if (!(isLayout && (name == "addVideoFile" || name == "sources")))
            (name == "uploadOverlay" ? overlaysColumn : availableColumn)->addWidget(button);
        else
            button->hide();
        connect(button, &QPushButton::clicked, &dialog, [&, fn] { run(fn); });
    };
    auto addFile = [&](QString path, bool overlay, bool place = true) {
        if (path.isEmpty())
            return;
        Json item;
        item.set("sourceKind", overlay ? "OVERLAY" : "FILE");
        if (overlay) {
            auto root = studio.store.root / "overlays";
            fs::create_directories(root);
            auto target = root / (QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString() +
                                  fs::path(path.toStdString()).extension().string());
            fs::copy_file(path.toStdString(), target);
            fs::permissions(target, fs::perms::owner_read | fs::perms::owner_write);
            path = QString::fromStdString(target.string());
        }
        item.set("sourceTarget", path.toStdString());
        if (!place)
            return;
        item.set("label", fs::path(path.toStdString()).filename().string());
        item.set("audioEnabled", !overlay);
        if (overlay)
            item.set("loop", true);
        auto selected = graphics->selectedItems();
        if (!isLayout && !overlay && !selected.isEmpty()) {
            auto slot = dynamic_cast<SourceTile*>(selected.first());
            if (slot && slot->source.get("sourceTarget").str().empty()) {
                auto old = slot->value();
                for (auto field : {"x", "y", "width", "height"})
                    item.set(field, old.get(field));
                delete slot;
            }
        }
        addSource(item);
    };
    view->fileDropped = [&](QString path) {
        run([&] {
            const auto ext = fs::path(path.toStdString()).extension().string();
            addFile(path,
                    ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".webp" || isLayout);
        });
    };
    action("addVideoFile",
           [&] { addFile(QFileDialog::getOpenFileName(&dialog, t("video")), false); });
    if (!isLayout) {
        auto fileRow = new QWidget(availableCard);
        auto line = new QHBoxLayout(fileRow);
        line->setContentsMargins(0, 0, 0, 0);
        auto path = new QLineEdit(fileRow);
        path->setObjectName("composerVideoPath");
        path->setPlaceholderText("/home/user/Videos/intro.mp4");
        auto add = new QPushButton(t("add"), fileRow);
        add->setAutoDefault(false);
        add->setObjectName("composerAddVideo");
        line->addWidget(path, 1);
        line->addWidget(add);
        availableColumn->addWidget(fileRow);
        connect(add, &QPushButton::clicked, &dialog,
                [&, path] { run([&] { addFile(path->text().trimmed(), false); }); });
    }
    action("uploadOverlay", [&] {
        addFile(QFileDialog::getOpenFileName(
                    &dialog, t("overlays"), {},
                    "Media (*.png *.jpg *.jpeg *.gif *.webp *.webm *.mov *.mp4)"),
                true, false);
    });
    action("addSlot", [&] {
        Json item;
        item.set("sourceKind", "FILE");
        item.set("sourceTarget", "");
        item.set("label", t("slot").toStdString());
        addSource(item);
    });
    if (!isLayout)
        sourcePanel->findChild<QPushButton*>("addSlot")->hide();
    auto addPickedSource = [&](Json picked) {
        for (auto node : graphics->items())
            if (auto tile = dynamic_cast<SourceTile*>(node);
                tile && tile->source.get("sourceTarget").str() == picked.get("target").str() &&
                tile->source.get("sourceKind").str() == picked.get("kind").str()) {
                graphics->clearSelection();
                tile->setSelected(true);
                return;
            }
        Json item;
        item.set("sourceKind", picked.get("kind"));
        item.set("sourceTarget", picked.get("target"));
        item.set("label", picked.get("label"));
        item.set("audioEnabled", true);
        auto selected = graphics->selectedItems();
        if (!isLayout && !selected.isEmpty()) {
            auto slot = dynamic_cast<SourceTile*>(selected.first());
            if (slot && slot->source.get("sourceTarget").str().empty()) {
                auto old = slot->value();
                for (auto field : {"x", "y", "width", "height"})
                    item.set(field, old.get(field));
                delete slot;
            }
        }
        addSource(item);
    };
    view->sourceDropped = [&](Json picked) { run([&] { addPickedSource(picked); }); };
    auto catalog = new QWidget(sourcePanel);
    auto catalogColumn = new QVBoxLayout(catalog);
    catalogColumn->setContentsMargins(0, 0, 0, 0);
    availableColumn->insertWidget(1, catalog);
    auto refreshCatalog = [&] {
        while (auto row = catalogColumn->takeAt(0)) {
            if (row->widget())
                row->widget()->deleteLater();
            delete row;
        }
        auto available = sources();
        for (size_t i = 0; i < available.size(); ++i) {
            auto picked = available.at(i);
            auto button = new SourceButton(QString::fromStdString(picked.get("label").str()),
                                           picked, catalog);
            button->setObjectName("catalogSource");
            button->setToolTip(QString::fromStdString(picked.get("label").str()));
            button->setText(button->fontMetrics().elidedText(button->text(), Qt::ElideRight, 140));
            button->setMinimumHeight(42);
            button->setIcon(
                studioIcon(picked.get("kind").str() == "MICROPHONE" ? "volume" : "screen"));
            connect(button, &QPushButton::clicked, &dialog,
                    [&, picked] { run([&] { addPickedSource(picked); }); });
            catalogColumn->addWidget(button);
        }
        if (!available.size())
            catalogColumn->addWidget(new QLabel(t("sourcesEmptyAvailable"), catalog));
    };
    if (!isLayout) {
        refreshCatalog();
        action("refreshSources", refreshCatalog);
    } else
        catalog->hide();
    auto overlayLibrary = new OverlayList(sourcePanel);
    overlayLibrary->setDragEnabled(true);
    overlayLibrary->setObjectName("composerOverlays");
    overlayLibrary->setViewMode(QListView::IconMode);
    overlayLibrary->setIconSize(QSize(72, 48));
    overlayLibrary->setMaximumHeight(180);
    auto refreshOverlays = [&] {
        overlayLibrary->clear();
        auto root = studio.store.root / "overlays";
        if (!fs::is_directory(root))
            return;
        for (const auto& file : fs::directory_iterator(root))
            if (file.is_regular_file()) {
                auto path = QString::fromStdString(file.path().string());
                auto row = new QListWidgetItem(QIcon(path),
                                               QString::fromStdString(file.path().stem().string()),
                                               overlayLibrary);
                row->setData(Qt::UserRole, path);
            }
    };
    overlaysColumn->insertWidget(0, new QLabel(t("overlays"), overlaysCard));
    auto overlayHint = new QLabel(t("overlaysHint"), overlaysCard);
    overlayHint->setObjectName("muted");
    overlayHint->setWordWrap(true);
    overlaysColumn->insertWidget(1, overlayHint);
    overlaysColumn->addWidget(overlayLibrary);
    connect(overlayLibrary, &QListWidget::itemClicked, &dialog, [&](QListWidgetItem* row) {
        Json item;
        item.set("sourceKind", "OVERLAY");
        item.set("sourceTarget", row->data(Qt::UserRole).toString().toStdString());
        item.set("label", row->text().toStdString());
        item.set("audioEnabled", false);
        item.set("loop", true);
        addSource(item);
    });
    auto overlayTimer = new QTimer(&dialog);
    connect(overlayTimer, &QTimer::timeout, &dialog, refreshOverlays);
    overlayTimer->start(1500);
    refreshOverlays();
    connect(layoutPicker, &QComboBox::currentIndexChanged, &dialog, [&] {
        auto id = layoutPicker->currentData().toLongLong();
        auto layout = id ? studio.store.get("scenes", id) : Json{};
        graphics->clear();
        width->setValue(layout.get("width").num(1920));
        height->setValue(layout.get("height").num(1080));
        if (isLayout) {
            scene = layout;
            scene.set("isLayout", true);
            name->setText(
                QString::fromStdString(layout.get("name").str(t("newLayout").toStdString())));
        }
        auto items = layout.get("items");
        for (size_t i = 0; i < items.size(); ++i)
            addSource(items.at(i));
        resolution->setCurrentText(QString::number(int(width->value())) + "x" +
                                   QString::number(int(height->value())));
    });
    auto inspector = new QWidget(&dialog);
    auto form = new QFormLayout(inspector);
    auto propertiesTitle = new QLabel(t("selectedSettingsDetails"), sourcePanel);
    propertiesTitle->setObjectName("sectionLabel");
    propertiesTitle->setWordWrap(true);
    sourceLayout->addWidget(propertiesTitle);
    sourceLayout->addWidget(inspector);
    sourceLayout->addStretch();
    QMap<QString, QDoubleSpinBox*> fields;
    auto selected = [&]() -> SourceTile* {
        auto list = graphics->selectedItems();
        return list.isEmpty() ? nullptr : dynamic_cast<SourceTile*>(list.first());
    };
    for (auto field : {"x", "y", "width", "height", "volume"}) {
        auto spin = new QDoubleSpinBox(inspector);
        spin->setObjectName(QString("source_") + field);
        spin->setRange(0, field == std::string("volume") ? 200 : 7680);
        spin->setDecimals(0);
        fields[field] = spin;
        form->addRow(field == std::string("width")    ? t("width")
                     : field == std::string("height") ? t("height")
                     : field == std::string("volume") ? t("sourceVolumes")
                                                      : QString(field),
                     spin);
        connect(spin, &QDoubleSpinBox::valueChanged, &dialog, [&, field](double value) {
            if (auto tile = selected()) {
                if (field == std::string("volume"))
                    tile->source.set("volume", value);
                else if (field == std::string("x") || field == std::string("y")) {
                    auto p = tile->pos();
                    if (field == std::string("x"))
                        p.setX(value);
                    else
                        p.setY(value);
                    tile->setPos(p);
                } else {
                    auto rect = tile->rect();
                    if (field == std::string("width"))
                        rect.setWidth(std::max(1.0, value));
                    else
                        rect.setHeight(std::max(1.0, value));
                    tile->setRect(rect);
                }
            }
        });
    }
    auto sound = new QCheckBox(t("soundOn"), inspector);
    form->addRow(sound);
    connect(sound, &QCheckBox::toggled, &dialog, [&](bool checked) {
        if (auto tile = selected())
            tile->source.set("audioEnabled", checked);
    });
    auto selectionChanged = [&] {
        inspector->setEnabled(selected() != nullptr);
        inspector->setVisible(selected() != nullptr);
        propertiesTitle->setVisible(selected() != nullptr);
        if (auto tile = selected()) {
            auto value = tile->value();
            for (auto it = fields.begin(); it != fields.end(); ++it) {
                it.value()->blockSignals(true);
                it.value()->setValue(it.key() == "volume"
                                         ? value.get("volume").num(100)
                                         : value.get(it.key().toStdString()).num());
                it.value()->blockSignals(false);
            }
            sound->blockSignals(true);
            sound->setChecked(value.get("audioEnabled").flag());
            sound->blockSignals(false);
        }
    };
    connect(graphics, &QGraphicsScene::selectionChanged, &dialog, selectionChanged);
    selectionChanged();
    auto moveLayer = [&](int direction) {
        auto tile = selected();
        if (!tile)
            return;
        QList<SourceTile*> ordered;
        for (auto node : graphics->items(Qt::AscendingOrder))
            if (auto layer = dynamic_cast<SourceTile*>(node))
                ordered.append(layer);
        const int index = ordered.indexOf(tile), target = index + direction;
        if (target < 0 || target >= ordered.size())
            return;
        ordered.swapItemsAt(index, target);
        for (int i = 0; i < ordered.size(); ++i)
            ordered[i]->setZValue(i);
    };
    action("bringForward", [&] { moveLayer(1); });
    action("sendBackward", [&] { moveLayer(-1); });
    action("fillCanvas", [&] {
        if (auto tile = selected()) {
            tile->setRect(0, 0, width->value(), height->value());
            tile->setPos(0, 0);
            selectionChanged();
        }
    });
    action("removeSource", [&] {
        if (auto tile = selected())
            delete tile;
    });
    auto layersHeader = new QFrame(layerPanel);
    layersHeader->setObjectName("card");
    auto layersHeading = new QVBoxLayout(layersHeader);
    auto layersTitle = new QLabel(t("layers"), layersHeader);
    layersTitle->setObjectName("cardTitle");
    layersHeading->addWidget(layersTitle);
    auto layersHint = new QLabel(t("layersHint"), layersHeader);
    layersHint->setObjectName("muted");
    layersHint->setWordWrap(true);
    layersHeading->addWidget(layersHint);
    layerColumn->addWidget(layersHeader);
    auto layers = new QListWidget(layerPanel);
    layers->setObjectName("sceneLayers");
    layers->setAccessibleName(t("layers"));
    layers->setMinimumHeight(160);
    layerColumn->addWidget(layers, 1);
    layerColumn->addWidget(layerActions);
    layerColumn->addStretch();
    auto refreshLayers = [&] {
        layers->blockSignals(true);
        layers->clear();
        for (auto item : graphics->items(Qt::DescendingOrder)) {
            if (auto tile = dynamic_cast<SourceTile*>(item)) {
                auto row = new QListWidgetItem(
                    QString::fromStdString(tile->source.get("label").str()), layers);
                row->setSelected(tile->isSelected());
            }
        }
        for (auto button : layerActions->findChildren<QPushButton*>())
            button->setEnabled(!graphics->selectedItems().isEmpty());
        layers->setVisible(layers->count() > 0);
        layerActions->setVisible(layers->count() > 0);
        layers->blockSignals(false);
    };
    connect(graphics, &QGraphicsScene::changed, &dialog, refreshLayers);
    connect(layers, &QListWidget::itemClicked, &dialog, [&](QListWidgetItem* row) {
        const int selectedRow = layers->row(row);
        int index = 0;
        for (auto item : graphics->items(Qt::DescendingOrder)) {
            if (auto tile = dynamic_cast<SourceTile*>(item)) {
                if (index++ == selectedRow) {
                    graphics->clearSelection();
                    tile->setSelected(true);
                    break;
                }
            }
        }
    });
    refreshLayers();
    if (!isLayout) {
        auto used = new QListWidget(sourcePanel);
        used->setObjectName("composerUsedSources");
        used->setMaximumHeight(120);
        auto usedCard = new QFrame(sourcePanel);
        usedCard->setObjectName("card");
        usedCard->setMinimumHeight(126);
        auto usedColumn = new QVBoxLayout(usedCard);
        usedColumn->addWidget(new QLabel(t("usedSources"), usedCard));
        auto emptyUsed = new QLabel(t("sourcesEmptyUsed"), usedCard);
        emptyUsed->setObjectName("muted");
        emptyUsed->setWordWrap(true);
        usedColumn->addWidget(emptyUsed);
        usedColumn->addWidget(used);
        sourceLayout->insertWidget(0, usedCard);
        auto refreshUsed = [&, used, emptyUsed] {
            used->clear();
            for (auto node : graphics->items(Qt::AscendingOrder))
                if (auto tile = dynamic_cast<SourceTile*>(node);
                    tile && !tile->source.get("sourceTarget").str().empty())
                    used->addItem(QString::fromStdString(tile->source.get("label").str()));
            emptyUsed->setVisible(used->count() == 0);
            used->setVisible(used->count() > 0);
        };
        connect(graphics, &QGraphicsScene::changed, &dialog, refreshUsed);
        refreshUsed();
    }
    auto fit = [&] {
        graphics->setSceneRect(0, 0, width->value(), height->value());
        view->fitInView(graphics->sceneRect(), Qt::KeepAspectRatio);
    };
    connect(width, &QDoubleSpinBox::valueChanged, &dialog, fit);
    connect(height, &QDoubleSpinBox::valueChanged, &dialog, fit);
    auto hint = new QLabel(t("composerKeys"), &dialog);
    hint->setWordWrap(true);
    hint->setObjectName("muted");
    mainLayout->addWidget(hint);
    connect(dialog.buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        run([&] {
            auto value = dialog.value();
            value.set("isLayout", isLayout);
            auto result = Json::array();
            for (auto item : graphics->items(Qt::AscendingOrder))
                if (auto tile = dynamic_cast<SourceTile*>(item))
                    result.push(tile->value());
            value.set("items", result);
            validateSceneForSave(value, studio.store.root);
            if (saved) {
                saved(value);
                dialog.accept();
            } else {
                auto id = saveScene(studio.store, value, scene.get("id").num());
                value.set("id", double(id));
                dialog.accept();
                sample();
            }
        });
    });
    QTimer::singleShot(0, &dialog, fit);
    dialog.exec();
    // La scena viene distrutta dopo i pannelli: non notificare selezione/geometria a widget già
    // liberati.
    disconnect(graphics, nullptr, &dialog, nullptr);
    return dialog.property("backRequested").toBool();
}
} // namespace orbis

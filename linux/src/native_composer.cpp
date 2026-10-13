#include "native_forms.hpp"
#include "native_window.hpp"
#include "scene_service.hpp"
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
#include <QSplitter>
#include <QTimer>
#include <QUuid>
#include <algorithm>

namespace orbis {
namespace {
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

    explicit CanvasView(QGraphicsScene* scene, QWidget* parent) : QGraphicsView(scene, parent) {
        setObjectName("sceneCanvas");
        setAcceptDrops(true);
        setRenderHint(QPainter::Antialiasing);
        setMinimumSize(600, 340);
        setBackgroundBrush(Qt::black);
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
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
        else
            QGraphicsView::dragEnterEvent(event);
    }

    void dragMoveEvent(QDragMoveEvent* event) override {
        if (event->mimeData()->hasUrls())
            event->acceptProposedAction();
        else
            QGraphicsView::dragMoveEvent(event);
    }

    void dropEvent(QDropEvent* event) override {
        for (const auto& url : event->mimeData()->urls())
            if (url.isLocalFile() && fileDropped)
                fileDropped(url.toLocalFile());
        event->acceptProposedAction();
    }
};
} // namespace

// L'editor nativo lavora su una copia: annullare non modifica layout o scene già utilizzati dalle
// dirette.
void NativeWindow::editScene(Json scene, std::function<void(Json)> saved) {
    const bool isLayout = scene.get("isLayout").flag();
    NativeForm dialog(this, t(isLayout ? "saveLayout" : "composeTitle"), scene);
    dialog.setObjectName("sceneEditor");
    dialog.resize(1120, 860);
    dialog.text("name", t(isLayout ? "layoutName" : "sceneName"));
    dialog.text("description", t("description"));
    auto width = dialog.number("width", t("width"), 1280, 16, 7680),
         height = dialog.number("height", t("height"), 720, 16, 7680);
    auto graphics = new QGraphicsScene(&dialog);
    graphics->setSceneRect(0, 0, width->value(), height->value());
    auto view = new CanvasView(graphics, &dialog);
    dialog.layout->insertWidget(1, view, 1);
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
    auto toolbar = new QWidget(&dialog);
    auto row = new QGridLayout(toolbar);
    dialog.layout->insertWidget(2, toolbar);
    auto action = [&](const char* key, std::function<void()> fn) {
        auto button = new QPushButton(t(key), toolbar);
        button->setObjectName(key);
        const int index = row->count();
        row->addWidget(button, index / 4, index % 4);
        connect(button, &QPushButton::clicked, &dialog, [&, fn] { run(fn); });
    };
    auto addFile = [&](QString path, bool overlay) {
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
    action("uploadOverlay", [&] {
        addFile(QFileDialog::getOpenFileName(
                    &dialog, t("overlays"), {},
                    "Media (*.png *.jpg *.jpeg *.gif *.webp *.webm *.mov *.mp4)"),
                true);
    });
    action("addSlot", [&] {
        Json item;
        item.set("sourceKind", "FILE");
        item.set("sourceTarget", "");
        item.set("label", t("slot").toStdString());
        addSource(item);
    });
    action("sources", [&] {
        auto rows = sources();
        NativeForm sourceDialog(&dialog, t("sources"));
        QList<QPair<QString, Json>> entries;
        for (size_t i = 0; i < rows.size(); ++i)
            entries.append({QString::fromStdString(rows.at(i).get("label").str()), rows.at(i)});
        if (entries.isEmpty())
            throw std::runtime_error("Nessuna sorgente");
        sourceDialog.choice("source", t("sources"), entries);
        connect(sourceDialog.buttons, &QDialogButtonBox::accepted, &sourceDialog, &QDialog::accept);
        if (sourceDialog.exec() != QDialog::Accepted)
            return;
        auto picked = sourceDialog.value().get("source");
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
    });
    auto inspector = new QWidget(&dialog);
    auto form = new QFormLayout(inspector);
    dialog.layout->insertWidget(3, inspector);
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
                    tile->source.set("volume", value / 100);
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
        if (auto tile = selected()) {
            auto value = tile->value();
            for (auto it = fields.begin(); it != fields.end(); ++it) {
                it.value()->blockSignals(true);
                it.value()->setValue(it.key() == "volume"
                                         ? value.get("volume").num(1) * 100
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
    action("bringForward", [&] {
        if (auto tile = selected())
            tile->setZValue(tile->zValue() + 1);
    });
    action("sendBackward", [&] {
        if (auto tile = selected())
            tile->setZValue(tile->zValue() - 1);
    });
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
    auto fit = [&] {
        graphics->setSceneRect(0, 0, width->value(), height->value());
        view->fitInView(graphics->sceneRect(), Qt::KeepAspectRatio);
    };
    connect(width, &QDoubleSpinBox::valueChanged, &dialog, fit);
    connect(height, &QDoubleSpinBox::valueChanged, &dialog, fit);
    auto hint = new QLabel(t("composerKeys"), &dialog);
    hint->setWordWrap(true);
    dialog.layout->insertWidget(4, hint);
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
}
} // namespace orbis

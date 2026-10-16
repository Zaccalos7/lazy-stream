#pragma once
#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QPainter>
#include <QSlider>
#include <QStyleOptionSlider>
#include <algorithm>

namespace orbis {
// Native equivalents of controls.css: the hit area and keyboard behavior remain Qt controls.
class ToggleSwitch : public QCheckBox {
  public:
    explicit ToggleSwitch(QWidget* parent = nullptr) : QCheckBox(parent) {
        setCursor(Qt::PointingHandCursor);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    QSize sizeHint() const override { return {38, 34}; }
  protected:
    bool hitButton(const QPoint& point) const override { return rect().contains(point); }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const bool light = qApp->property("orbisTheme").toString() == "light";
        p.setOpacity(isEnabled() ? 1 : .45);
        p.setPen(Qt::NoPen);
        p.setBrush(isChecked() ? QColor("#0a84ff") : QColor(light ? "#d5d5d8" : "#5a5a60"));
        p.drawRoundedRect(QRectF(2, (height()-20)/2., 34, 20), 10, 10);
        p.setBrush(Qt::white);
        p.drawEllipse(QRectF(isChecked() ? 18 : 4, (height()-16)/2., 16, 16));
        if (hasFocus()) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor("#409cff"), 1));
            p.drawRoundedRect(QRectF(0, (height()-24)/2., 38, 24), 12, 12);
        }
    }
};

class MeterRing : public QWidget {
    Q_OBJECT
    double current = -1;
    QString suffix;
  public:
    explicit MeterRing(QString unit, QWidget* parent = nullptr) : QWidget(parent), suffix(unit) {
        setMinimumSize(130, 130);
    }
    void setValue(double value) { current = value; update(); }
    double value() const { return current; }
    QSize sizeHint() const override { return {150, 150}; }
  protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const bool light = qApp->property("orbisTheme").toString() == "light";
        const auto side = std::min(width(), height()) - 20;
        QRectF arc((width()-side)/2., (height()-side)/2., side, side);
        p.setPen(QPen(QColor(light ? "#e4e4e8" : "#404044"), 10));
        p.drawEllipse(arc);
        const QColor color(current < 40 ? "#30d158" : current < 75 ? "#ffd60a" : "#ff453a");
        p.setPen(QPen(color, 10, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(arc, 90*16, -int(std::clamp(current, 0., 100.) * 3.6 * 16));
        p.setPen(QColor(light ? "#1d1d1f" : "#f5f5f7"));
        auto font = p.font(); font.setPixelSize(25); font.setWeight(QFont::DemiBold); p.setFont(font);
        p.drawText(rect(), Qt::AlignCenter, current < 0 ? QString::fromUtf8("–") : QString::number(current, 'f', 1) + suffix);
    }
};

class VolumeSlider : public QSlider {
  public:
    explicit VolumeSlider(QWidget* parent) : QSlider(Qt::Horizontal, parent) {
        setRange(0, 200); setSingleStep(5); setMinimumHeight(22);
    }
  protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
        const bool muted = property("muted").toBool();
        const QColor color(muted ? "#98989d" : value() <= 100 ? "#30d158" : value() < 150 ? "#ffd60a" : "#ff453a");
        const qreal x = 8 + (width()-16) * value() / 200.;
        p.setPen(Qt::NoPen); p.setBrush(QColor("#404044"));
        p.drawRoundedRect(QRectF(8, height()/2.-3, width()-16, 6), 3, 3);
        p.setBrush(color); p.drawRoundedRect(QRectF(8, height()/2.-3, x-8, 6), 3, 3);
        p.setPen(QPen(QColor("#85858b"), 2)); p.drawLine(QPointF(width()/2., height()/2.-7), QPointF(width()/2., height()/2.+7));
        p.setPen(QPen(color, 2)); p.setBrush(qApp->palette().color(QPalette::Window));
        p.drawEllipse(QPointF(x, height()/2.), 7, 7);
    }
};
} // namespace orbis

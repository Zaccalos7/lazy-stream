#pragma once
#include <QApplication>
#include <QElapsedTimer>
#include <QGraphicsDropShadowEffect>
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <algorithm>

namespace orbis {
class DashboardTile : public QPushButton {
    QGraphicsDropShadowEffect* shadow;

    void restyleShadow(bool hover) {
        const bool light = qApp->property("orbisTheme").toString() == "light";
        shadow->setColor(QColor(0, 0, 0, light ? (hover ? 15 : 8) : (hover ? 102 : 76)));
        shadow->setOffset(0, hover ? 8 : 2);
        shadow->setBlurRadius(hover ? 24 : 8);
    }

  protected:
    void enterEvent(QEnterEvent* event) override {
        QPushButton::enterEvent(event);
        restyleShadow(true);
    }

    void leaveEvent(QEvent* event) override {
        QPushButton::leaveEvent(event);
        restyleShadow(false);
    }

  public:
    explicit DashboardTile(QWidget* parent) : QPushButton(parent) {
        shadow = new QGraphicsDropShadowEffect(this);
        setGraphicsEffect(shadow);
        restyleShadow(false);
    }
};

// Equivalente di repeat(auto-fill, minmax(250px, 1fr)): anche le sezioni con una sola
// scheda conservano le stesse colonne delle altre sezioni della dashboard.
class DashboardGrid : public QWidget {
    QList<QPushButton*> cards;

    int arrange(int available, bool apply) const {
        const int columns = std::max(1, (available + 12) / 262);
        const int width = std::max(1, (available - (columns - 1) * 12) / columns);
        int y = 0;
        for (int first = 0; first < cards.size(); first += columns) {
            int height = 0;
            for (int i = first; i < std::min(first + columns, int(cards.size())); ++i)
                height = std::max(height, cards[i]->layout()->totalHeightForWidth(width));
            height = std::max(145, height);
            for (int i = first; apply && i < std::min(first + columns, int(cards.size())); ++i) {
                const int column = i - first;
                const int x = layoutDirection() == Qt::RightToLeft
                                  ? available - column * (width + 12) - width
                                  : column * (width + 12);
                cards[i]->setGeometry(x, y, width, height);
            }
            y += height + 12;
        }
        return std::max(0, y - 12);
    }

  protected:
    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        const int needed = arrange(event->size().width(), true);
        if (minimumHeight() != needed || maximumHeight() != needed) {
            setFixedHeight(needed);
            updateGeometry();
        }
    }

  public:
    explicit DashboardGrid(QWidget* parent) : QWidget(parent) {
        setObjectName("dashboardGrid");
        auto policy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        setSizePolicy(policy);
        setMinimumWidth(250);
    }

    void addCard(QPushButton* card) {
        cards.append(card);
        updateGeometry();
    }

    int heightForWidth(int width) const override {
        return arrange(width, false);
    }

    QSize sizeHint() const override {
        return {250, heightForWidth(250)};
    }
};

// Il banner originale pulsa ogni tre secondi e sospende il movimento al passaggio del mouse.
class CoffeeButton : public QPushButton {
    QElapsedTimer elapsed;
    QGraphicsDropShadowEffect* glow;
    QTimer animation;

  protected:
    void paintEvent(QPaintEvent* event) override {
        QPushButton::paintEvent(event);
        if (underMouse())
            return;
        const double progress = (elapsed.elapsed() % 3000) / 3000.0;
        if (progress > 0.25)
            return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath clip;
        clip.addRoundedRect(QRectF(rect()), 18, 18);
        painter.setClipPath(clip);
        const qreal x = (-1 + 12 * progress) * width();
        QLinearGradient shine(x, 0, x + width() / 2.0, 0);
        shine.setColorAt(0, Qt::transparent);
        shine.setColorAt(0.5, QColor(255, 149, 0, 50));
        shine.setColorAt(1, Qt::transparent);
        painter.fillRect(rect(), shine);
    }

  public:
    explicit CoffeeButton(const QString& text, QWidget* parent) : QPushButton(text, parent) {
        setObjectName("coffeeBanner");
        setCursor(Qt::PointingHandCursor);
        glow = new QGraphicsDropShadowEffect(this);
        glow->setOffset(0, 0);
        glow->setBlurRadius(10);
        setGraphicsEffect(glow);
        elapsed.start();
        animation.setInterval(50);
        connect(&animation, &QTimer::timeout, this, [this] {
            if (!isVisible() || underMouse())
                return;
            const double phase = (elapsed.elapsed() % 3000) / 3000.0;
            glow->setColor(QColor(255, 149, 0, int(64 * (1 - std::abs(2 * phase - 1)))));
            update();
        });
        animation.start();
    }
};
} // namespace orbis

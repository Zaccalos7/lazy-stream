#include "native_theme.hpp"
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyleFactory>

namespace orbis {
// Le icone sono vettoriali e rasterizzate anche a 2x per gli schermi ad alta densità.
QIcon studioIcon(const QString& name) {
    QIcon icon;
    for (int scale : {1, 2, 3}) {
        QPixmap pixmap(24 * scale, 24 * scale);
        pixmap.fill(Qt::transparent);
        pixmap.setDevicePixelRatio(scale);
        QPainter p(&pixmap);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor("#c4c4cc"), 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (name == "dashboard") {
            for (int x : {4, 14})
                for (int y : {4, 14})
                    p.drawRoundedRect(QRectF(x, y, 6, 6), 1.4, 1.4);
        } else if (name == "live" || name == "channels") {
            p.drawEllipse(QPointF(12, 12), 2, 2);
            p.drawArc(QRectF(6, 6, 12, 12), -55 * 16, 110 * 16);
            p.drawArc(QRectF(6, 6, 12, 12), 125 * 16, 110 * 16);
            p.drawArc(QRectF(2, 2, 20, 20), -45 * 16, 90 * 16);
            p.drawArc(QRectF(2, 2, 20, 20), 135 * 16, 90 * 16);
        } else if (name == "preview" || name == "play") {
            if (name == "preview")
                p.drawRoundedRect(QRectF(3, 4, 18, 14), 2, 2);
            QPainterPath path;
            path.moveTo(10, 8);
            path.lineTo(16, 11.5);
            path.lineTo(10, 15);
            path.closeSubpath();
            p.drawPath(path);
            if (name == "preview")
                p.drawLine(8, 21, 16, 21);
        } else if (name == "layouts" || name == "scenes") {
            p.drawRoundedRect(QRectF(3, 4, 18, 16), 2, 2);
            p.drawLine(3, 10, 21, 10);
            p.drawLine(14, 10, 14, 20);
        } else if (name == "activity") {
            QPolygonF points;
            points << QPointF(2, 13) << QPointF(6, 13) << QPointF(9, 5) << QPointF(14, 20)
                   << QPointF(18, 10) << QPointF(22, 10);
            p.drawPolyline(points);
        } else if (name == "history") {
            p.drawEllipse(QRectF(4, 4, 16, 16));
            p.drawLine(12, 7, 12, 12);
            p.drawLine(12, 12, 16, 14);
        } else if (name == "manual" || name == "playlist") {
            p.drawRoundedRect(QRectF(4, 3, 16, 18), 2, 2);
            for (int y : {8, 12, 16})
                p.drawLine(8, y, 16, y);
        } else if (name == "plus") {
            p.drawLine(5, 12, 19, 12);
            p.drawLine(12, 5, 12, 19);
        } else {
            for (int x : {6, 12, 18}) {
                p.drawLine(x, 4, x, 20);
                p.setBrush(QColor("#262628"));
                p.drawEllipse(QPointF(x, x == 12 ? 15 : 9), 2.5, 2.5);
            }
        }
        p.end();
        icon.addPixmap(pixmap);
    }
    return icon;
}

// La palette copre anche i controlli nativi non stilizzati e i dialoghi modali.
void applyNativeTheme() {
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#1a1a1c"));
    palette.setColor(QPalette::WindowText, QColor("#f5f5f7"));
    palette.setColor(QPalette::Base, QColor("#262628"));
    palette.setColor(QPalette::AlternateBase, QColor("#2b2b2e"));
    palette.setColor(QPalette::Text, QColor("#f5f5f7"));
    palette.setColor(QPalette::Button, QColor("#323234"));
    palette.setColor(QPalette::ButtonText, QColor("#f5f5f7"));
    palette.setColor(QPalette::Highlight, QColor("#007aff"));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::ToolTipBase, QColor("#323234"));
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Link, QColor("#9db8ff"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#738198"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#738198"));
    qApp->setPalette(palette);
    auto font = qApp->font();
    font.setPointSize(10);
    qApp->setFont(font);
    auto stylesheet = QString::fromUtf8(R"(
        QWidget { color: #f5f5f7; }
        QMainWindow, QDialog, QWizard, QWidget#workspace { background: #1a1a1c; }
        QWidget#sidebar { background: #232325; border-right: 1px solid #414145; }
        QLabel { background: transparent; }
        QLabel#brand { font-size: 22px; font-weight: 700; color: #f5f8ff; }
        QLabel#brandMark { background: #007aff; border-radius: 12px; font-size: 23px; font-weight: 700; color: white; }
        QLabel#muted, QLabel#sectionLabel { color: #b2b2ba; }
        QLabel#sectionLabel { font-size: 11px; font-weight: 600; }
        QLabel#pageTitle { font-size: 28px; font-weight: 700; }
        QLabel#heroTitle { font-size: 24px; font-weight: 700; }
        QLabel#metricValue { font-size: 28px; font-weight: 600; }
        QLabel#statusPill { color: #92e4cf; background: #19382f; padding: 7px 12px; border-radius: 12px; }
        QListWidget#navigation { background: transparent; border: none; outline: none; }
        QListWidget#navigation::item { padding: 10px 12px; margin: 3px 0; border-radius: 8px; color: #b9b9c0; }
        QListWidget#navigation::item:hover { background: #323234; color: white; }
        QListWidget#navigation::item:selected { background: #007aff; color: #f4f7ff; }
        QFrame#hero { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #263759, stop:1 #1a253b); border: 1px solid #3b5075; border-radius: 16px; }
        QFrame#card { background: #262628; border: 1px solid #414145; border-radius: 12px; }
        QFrame#hero QLabel, QFrame#card QLabel { border: none; background: transparent; }
        QPushButton, QToolButton { background: #323234; border: 1px solid #505055; border-radius: 7px; padding: 9px 14px; }
        QPushButton:hover, QToolButton:hover { background: #3e3e42; border-color: #6587c4; }
        QPushButton:pressed, QToolButton:pressed { background: #354f7d; }
        QPushButton:focus, QToolButton:focus { border: 1px solid #a6bbff; }
        QPushButton:disabled, QToolButton:disabled { background: #192231; color: #738198; border-color: #293447; }
        QPushButton[primary="true"], QPushButton:default { background: #007aff; color: white; border-color: #3396ff; font-weight: 600; }
        QPushButton[primary="true"]:hover, QPushButton:default:hover { background: #0a84ff; }
        QPushButton[primary="true"]:pressed { background: #0066cc; }
        QPushButton[primary="true"]:disabled { background: #303c66; color: #8692b3; border-color: #394668; }
        QPushButton[danger="true"] { color: #ffb0b8; border-color: #71424e; }
        QPushButton[card="true"] { text-align: left; background: #262628; border-radius: 12px; padding: 20px; font-weight: 600; }
        QPushButton[card="true"]:hover { background: #323234; border-color: #6587c4; }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateTimeEdit { background: #262628; border: 1px solid #505055; border-radius: 6px; padding: 7px 9px; selection-background-color: #245987; }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus { border-color: #839fff; }
        QComboBox { padding-right: 26px; }
        QComboBox::drop-down { width: 24px; border: none; }
        QComboBox QAbstractItemView { background: #323234; border: 1px solid #465b80; selection-background-color: #245987; padding: 5px; }
        QTextEdit, QPlainTextEdit, QTextBrowser, QListWidget, QTreeView { background: #262628; border: 1px solid #414145; border-radius: 8px; padding: 6px; }
        QScrollArea { border: none; background: transparent; }
        QTableWidget { background: #262628; alternate-background-color: #2b2b2e; border: 1px solid #414145; border-radius: 10px; selection-background-color: #245987; gridline-color: #38383c; }
        QTableWidget::item { padding: 8px; border-bottom: 1px solid #38383c; }
        QHeaderView::section { background: #323234; color: #c4c4cc; border: none; border-bottom: 1px solid #505055; padding: 12px 8px; font-weight: 600; }
        QTableCornerButton::section { background: #323234; border: none; }
        QMenu { background: #2b2b2e; border: 1px solid #405372; padding: 6px; }
        QMenu::item { padding: 9px 24px; border-radius: 5px; }
        QMenu::item:selected { background: #245987; }
        QMenu::separator { height: 1px; background: #505055; margin: 4px; }
        QToolTip { background: #323234; color: white; border: 1px solid #52688d; padding: 6px; }
        QGroupBox { border: 1px solid #505055; border-radius: 9px; margin-top: 16px; padding-top: 16px; }
        QGroupBox::title { subcontrol-origin: margin; padding: 0 6px; color: #b5c5df; }
        QTabWidget::pane { border: 1px solid #505055; border-radius: 7px; }
        QTabBar::tab { background: #262628; padding: 10px 18px; border-bottom: 2px solid transparent; }
        QTabBar::tab:selected { background: #323234; border-bottom-color: #839fff; }
        QCheckBox { spacing: 8px; padding: 3px; }
        QProgressBar { background: #262628; border: 1px solid #414145; border-radius: 5px; text-align: center; min-height: 8px; }
        QProgressBar::chunk { background: #6484ef; border-radius: 4px; }
        QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
        QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }
        QScrollBar::handle { background: #58585f; border-radius: 5px; min-height: 30px; min-width: 30px; }
        QScrollBar::handle:hover { background: #607aa1; }
        QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
        QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
        QStatusBar { background: #232325; color: #b2b2ba; border-top: 1px solid #414145; }
        QStatusBar::item { border: none; }
        QLabel#livePreview { background: #080d16; border: 1px solid #505055; border-radius: 12px; color: #b2b2ba; }
        QFrame#pageHeading { background: #262628; border: 1px solid #414145; border-radius: 12px; }
        QLabel#emptyState { background: #232325; border: 1px dashed #58585f; border-radius: 12px; color: #b9b9c0; padding: 24px; }
        QLabel#cardTitle { font-size: 16px; font-weight: 600; }
        QPushButton[card="true"] { border-top: 3px solid #007aff; padding: 0; }
        QPushButton[card="true"] QLabel { background: transparent; border: none; }
        QPushButton[compact="true"] { padding: 6px 10px; }
        QListWidget#wizardProfile, QListWidget#wizardChannel { background: transparent; border: none; }
        QListWidget#wizardProfile::item, QListWidget#wizardChannel::item { background: #323234; border: 1px solid #505055; border-radius: 9px; padding: 14px; margin-bottom: 8px; }
        QListWidget#wizardProfile::item:selected, QListWidget#wizardChannel::item:selected { background: #193f66; border: 2px solid #0a84ff; }
        QListWidget#wizardProfile::item:hover, QListWidget#wizardChannel::item:hover { border-color: #8d8d94; }
    )");
    qApp->setStyleSheet(stylesheet);
}
} // namespace orbis

#include "native_theme.hpp"
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QTransform>
#include <algorithm>
#include <cmath>

namespace orbis {
namespace {
constexpr double pi = 3.14159265358979323846;

// Legge i parametri di un path SVG: i numeri possono essere attaccati ("1.714-2.1") e i comandi
// ripetuti senza essere riscritti ("h1v2").
struct SvgReader {
    const QString& data;
    int index = 0;

    void skip() {
        while (index < data.size() && (data.at(index).isSpace() || data.at(index) == ','))
            ++index;
    }

    QChar peek() {
        skip();
        return index < data.size() ? data.at(index) : QChar();
    }

    double number() {
        skip();
        const int start = index;
        if (index < data.size() && (data.at(index) == '-' || data.at(index) == '+'))
            ++index;
        bool dot = false, exponent = false;
        while (index < data.size()) {
            const QChar c = data.at(index);
            if (c.isDigit()) {
                ++index;
                continue;
            }
            if (c == '.' && !dot && !exponent) {
                dot = true;
                ++index;
                continue;
            }
            if ((c == 'e' || c == 'E') && !exponent) {
                exponent = true;
                ++index;
                if (index < data.size() && (data.at(index) == '-' || data.at(index) == '+'))
                    ++index;
                continue;
            }
            break;
        }
        return data.mid(start, index - start).toDouble();
    }
};

double vectorAngle(double ux, double uy, double vx, double vy) {
    const double dot = ux * vx + uy * vy;
    const double length = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
    double angle = std::acos(std::clamp(length > 0 ? dot / length : 1.0, -1.0, 1.0));
    if (ux * vy - uy * vx < 0)
        angle = -angle;
    return angle;
}

// Un arco SVG (parametrizzazione per estremi) diventa curve di Bézier, come prescrive la specifica.
void arcTo(QPainterPath& path, double x1, double y1, double rx, double ry, double rotationDegrees,
           bool largeArc, bool sweep, double x2, double y2) {
    if (rx == 0 || ry == 0) {
        path.lineTo(x2, y2);
        return;
    }
    rx = std::abs(rx);
    ry = std::abs(ry);
    const double phi = rotationDegrees * pi / 180.0, cosPhi = std::cos(phi), sinPhi = std::sin(phi);
    const double dx = (x1 - x2) / 2.0, dy = (y1 - y2) / 2.0;
    const double x1p = cosPhi * dx + sinPhi * dy, y1p = -sinPhi * dx + cosPhi * dy;
    const double lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if (lambda > 1) {
        const double scale = std::sqrt(lambda);
        rx *= scale;
        ry *= scale;
    }
    const double sign = largeArc != sweep ? 1.0 : -1.0;
    const double numerator = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p;
    const double denominator = rx * rx * y1p * y1p + ry * ry * x1p * x1p;
    const double coefficient =
        sign * std::sqrt(std::max(0.0, denominator > 0 ? numerator / denominator : 0.0));
    const double cxp = coefficient * (rx * y1p / ry), cyp = coefficient * (-ry * x1p / rx);
    const double cx = cosPhi * cxp - sinPhi * cyp + (x1 + x2) / 2.0;
    const double cy = sinPhi * cxp + cosPhi * cyp + (y1 + y2) / 2.0;
    double theta1 = vectorAngle(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry);
    double delta =
        vectorAngle((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry);
    if (!sweep && delta > 0)
        delta -= 2 * pi;
    else if (sweep && delta < 0)
        delta += 2 * pi;
    const int segments = std::max(1, int(std::ceil(std::abs(delta) / (pi / 2))));
    const double step = delta / segments, tangent = (4.0 / 3.0) * std::tan(step / 4);
    for (int segment = 0; segment < segments; ++segment) {
        const double cosT1 = std::cos(theta1), sinT1 = std::sin(theta1);
        const double theta2 = theta1 + step, cosT2 = std::cos(theta2), sinT2 = std::sin(theta2);
        const double e1x = cx + rx * cosPhi * cosT1 - ry * sinPhi * sinT1;
        const double e1y = cy + rx * sinPhi * cosT1 + ry * cosPhi * sinT1;
        const double e2x = cx + rx * cosPhi * cosT2 - ry * sinPhi * sinT2;
        const double e2y = cy + rx * sinPhi * cosT2 + ry * cosPhi * sinT2;
        const double d1x = -rx * cosPhi * sinT1 - ry * sinPhi * cosT1;
        const double d1y = -rx * sinPhi * sinT1 + ry * cosPhi * cosT1;
        const double d2x = -rx * cosPhi * sinT2 - ry * sinPhi * cosT2;
        const double d2y = -rx * sinPhi * sinT2 + ry * cosPhi * cosT2;
        path.cubicTo(e1x + tangent * d1x, e1y + tangent * d1y, e2x - tangent * d2x,
                     e2y - tangent * d2y, e2x, e2y);
        theta1 = theta2;
    }
}

// Costruisce una QPainterPath dai dati di un path SVG (comandi assoluti e relativi).
QPainterPath pathFromSvg(const QString& data) {
    QPainterPath path;
    SvgReader reader{data};
    QChar command, previous;
    double x = 0, y = 0, startX = 0, startY = 0, controlX = 0, controlY = 0;
    while (true) {
        const QChar next = reader.peek();
        if (next.isNull())
            break;
        if (next.isLetter()) {
            command = next;
            ++reader.index;
        } else if (command.isNull() || command.toUpper() == QChar('Z')) {
            break;
        }
        const bool relative = command.isLower();
        switch (command.toUpper().unicode()) {
        case 'M': {
            double nx = reader.number(), ny = reader.number();
            if (relative) {
                nx += x;
                ny += y;
            }
            x = nx;
            y = ny;
            startX = x;
            startY = y;
            path.moveTo(x, y);
            command = relative ? QChar('l') : QChar('L');
            break;
        }
        case 'L': {
            double nx = reader.number(), ny = reader.number();
            if (relative) {
                nx += x;
                ny += y;
            }
            x = nx;
            y = ny;
            path.lineTo(x, y);
            break;
        }
        case 'H': {
            double nx = reader.number();
            if (relative)
                nx += x;
            x = nx;
            path.lineTo(x, y);
            break;
        }
        case 'V': {
            double ny = reader.number();
            if (relative)
                ny += y;
            y = ny;
            path.lineTo(x, y);
            break;
        }
        case 'C': {
            double x1 = reader.number(), y1 = reader.number(), x2 = reader.number(),
                   y2 = reader.number(), nx = reader.number(), ny = reader.number();
            if (relative) {
                x1 += x;
                y1 += y;
                x2 += x;
                y2 += y;
                nx += x;
                ny += y;
            }
            path.cubicTo(x1, y1, x2, y2, nx, ny);
            controlX = x2;
            controlY = y2;
            x = nx;
            y = ny;
            break;
        }
        case 'S': {
            double x2 = reader.number(), y2 = reader.number(), nx = reader.number(),
                   ny = reader.number();
            if (relative) {
                x2 += x;
                y2 += y;
                nx += x;
                ny += y;
            }
            const QChar prior = previous.toUpper();
            const double x1 = (prior == QChar('C') || prior == QChar('S')) ? 2 * x - controlX : x;
            const double y1 = (prior == QChar('C') || prior == QChar('S')) ? 2 * y - controlY : y;
            path.cubicTo(x1, y1, x2, y2, nx, ny);
            controlX = x2;
            controlY = y2;
            x = nx;
            y = ny;
            break;
        }
        case 'Q': {
            double x1 = reader.number(), y1 = reader.number(), nx = reader.number(),
                   ny = reader.number();
            if (relative) {
                x1 += x;
                y1 += y;
                nx += x;
                ny += y;
            }
            path.quadTo(x1, y1, nx, ny);
            controlX = x1;
            controlY = y1;
            x = nx;
            y = ny;
            break;
        }
        case 'T': {
            double nx = reader.number(), ny = reader.number();
            if (relative) {
                nx += x;
                ny += y;
            }
            const QChar prior = previous.toUpper();
            const double x1 = (prior == QChar('Q') || prior == QChar('T')) ? 2 * x - controlX : x;
            const double y1 = (prior == QChar('Q') || prior == QChar('T')) ? 2 * y - controlY : y;
            path.quadTo(x1, y1, nx, ny);
            controlX = x1;
            controlY = y1;
            x = nx;
            y = ny;
            break;
        }
        case 'A': {
            const double rx = reader.number(), ry = reader.number(), rotation = reader.number(),
                         large = reader.number(), sweep = reader.number(), nx = reader.number(),
                         ny = reader.number();
            double ex = nx, ey = ny;
            if (relative) {
                ex += x;
                ey += y;
            }
            arcTo(path, x, y, rx, ry, rotation, large != 0, sweep != 0, ex, ey);
            x = ex;
            y = ey;
            break;
        }
        case 'Z':
            path.closeSubpath();
            x = startX;
            y = startY;
            break;
        default:
            break;
        }
        previous = command;
    }
    return path;
}

// I marchi delle piattaforme, come li disegna Windows (Pages/PageHelpers.cs).
const char* const twitchMark =
    "M11.571 4.714h1.715v5.143H11.57zm4.715 0H18v5.143h-1.714zM6 0L1.714 "
    "4.286v15.428h5.143V24l4.286-4.286h3.428L22.286 "
    "12V0zm14.571 11.143l-3.428 3.428h-3.429l-3 3v-3H6.857V1.714h13.714Z";
const char* const youtubeMark = "M23.498 6.186a3.016 3.016 0 0 0-2.122-2.136C19.505 3.545 12 3.545 "
                                "12 3.545s-7.505 0-9.377.505A3.017 3.017 0 0 0 "
                                ".502 6.186C0 8.07 0 12 0 12s0 3.93.502 5.814a3.016 3.016 0 0 0 "
                                "2.122 2.136c1.871.505 9.376.505 9.376.505s7.505 "
                                "0 9.377-.505a3.015 3.015 0 0 0 2.122-2.136C24 15.93 24 12 24 "
                                "12s0-3.93-.502-5.814zM9.545 15.568V8.432L15.818 "
                                "12l-6.273 3.568z";
const char* const kickMark =
    "M1.333 0h8v5.333H12V2.667h2.667V0h8v8H20v2.667h-2.667v2.666H20V16h2.667v8h-8v-2.667H12v-2."
    "666H9.333V24h-8Z";
const char* const facebookMark =
    "M0 0h24v24H0zM3.75 3.75v16.5h10.8v-5.03H8.83V8.82h11.48V3.75zM9.5 9.49v5.03h5.73v5.73h5."
    "08V9.49z";
const char* const tiktokMark = "M12.525.02c1.31-.02 2.61-.01 3.91-.02.08 1.53.63 3.09 1.75 4.17 "
                               "1.12 1.11 2.7 1.62 4.24 1.79v4.03c-1.44-.05-2.89-"
                               ".35-4.2-.97-.57-.26-1.1-.59-1.62-.93-.01 2.92.01 5.84-.02 8.75-.08 "
                               "1.4-.54 2.79-1.35 3.94-1.31 1.92-3.58 3.17-"
                               "5.91 "
                               "3.21-1.43.08-2.86-.31-4.08-1.03-2.02-1.19-3.44-3.37-3.65-5.71-.02-."
                               "5-.03-1-.01-1.49.18-1.9 1.12-3.72 2.58-"
                               "4.96 1.66-1.44 3.98-2.13 6.15-1.72.02 1.48-.04 2.96-.04 "
                               "4.44-.99-.32-2.15-.23-3.02.37-.63.41-1.11 1.04-1.36 1.75-"
                               ".21.51-.15 1.07-.14 1.61.24 1.64 1.82 3.02 3.5 2.87 1.12-.01 "
                               "2.19-.66 2.77-1.61.19-.33.4-.67.41-1.06.1-1.79.06-"
                               "3.57.07-5.36.01-4.03-.01-8.05.02-12.07z";
const char* const rumbleMark = "M14.4528 13.5458c.8064-.6542.9297-1.8381.2756-2.6445a1.8802 1.8802 "
                               "0 0 0-.2756-.2756 21.2127 21.2127 0 0 0-4."
                               "3121-2.776c-1.066-.51-2.256.2-2.4261 1.414a23.5226 23.5226 0 0 "
                               "0-.14 5.5021c.116 1.23 1.292 1.964 2.372 1.492a"
                               "19.6285 19.6285 0 0 0 4.5062-2.704v-.008zm6.9322-5.4002c2.0335 "
                               "2.228 2.0396 5.637.014 7.8723A26.1487 26.1487 0 0 "
                               "1 8.2946 "
                               "23.846c-2.6848.6713-5.4168-.914-6.1662-3.5781-1.524-5.2002-1.3-11."
                               "0803.17-16.3045.772-2.744 3.3521-4."
                               "4661 6.0102-3.832 4.9242 1.174 9.5443 4.196 13.0764 8.0121v.002z";
const char* const xMark =
    "M14.234 10.162 22.977 0h-2.072l-7.591 8.824L7.251 0H.258l9.168 13.343L.258 "
    "24H2.33l8.016-9.318L16.749 24h6.993z"
    "m-2.837 3.299-.929-1.329L3.076 1.56h3.182l5.965 8.532.929 1.329 7.754 11.09h-3.182z";
const char* const playMark = "M8 5v14l11-7z";
} // namespace

// Le icone sono vettoriali e rasterizzate anche a 2x per gli schermi ad alta densità.
QIcon studioIcon(const QString& name, const QColor& color) {
    const QColor ink = color.isValid() ? color : qApp->palette().color(QPalette::WindowText);
    QIcon icon;
    for (int scale : {1, 2, 3}) {
        QPixmap pixmap(24 * scale, 24 * scale);
        pixmap.fill(Qt::transparent);
        pixmap.setDevicePixelRatio(scale);
        QPainter p(&pixmap);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(ink, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const QColor inkFill = ink;
        if (name == "menu") {
            for (int y : {6, 12, 18})
                p.drawLine(3, y, 21, y);
        } else if (name == "home") {
            QPolygonF roof;
            roof << QPointF(2, 11) << QPointF(12, 2) << QPointF(22, 11);
            p.drawPolyline(roof);
            QPainterPath house;
            house.moveTo(5, 9);
            house.lineTo(5, 21);
            house.lineTo(10, 21);
            house.lineTo(10, 14);
            house.lineTo(14, 14);
            house.lineTo(14, 21);
            house.lineTo(19, 21);
            house.lineTo(19, 9);
            p.drawPath(house);
        } else if (name == "info") {
            p.drawEllipse(QRectF(3, 3, 18, 18));
            p.drawPoint(12, 7);
            p.drawLine(12, 11, 12, 17);
        } else if (name == "dashboard") {
            for (int x : {4, 14})
                for (int y : {4, 14})
                    p.drawRoundedRect(QRectF(x, y, 6, 6), 1.4, 1.4);
        } else if (name == "light") {
            p.drawEllipse(QPointF(12, 12), 4, 4);
            for (int i = 0; i < 8; ++i) {
                const double angle = i * pi / 4;
                p.drawLine(QPointF(12 + 7 * std::cos(angle), 12 + 7 * std::sin(angle)),
                           QPointF(12 + 10 * std::cos(angle), 12 + 10 * std::sin(angle)));
            }
        } else if (name == "dark") {
            QPainterPath moon;
            moon.moveTo(16, 3);
            moon.cubicTo(3, 0, 0, 18, 12, 21);
            moon.cubicTo(17, 22, 21, 18, 21, 14);
            moon.cubicTo(12, 18, 7, 8, 16, 3);
            p.drawPath(moon);
        } else if (name == "video") {
            p.drawRect(QRectF(3, 6, 12, 12));
            QPolygonF camera;
            camera << QPointF(15, 10) << QPointF(21, 6) << QPointF(21, 18) << QPointF(15, 14);
            p.drawPolyline(camera);
        } else if (name == "screen") {
            p.drawRect(QRectF(2, 4, 20, 14));
            p.drawLine(12, 18, 12, 21);
            p.drawLine(7, 21, 17, 21);
        } else if (name == "channelSettings" || name == "live" || name == "channels") {
            p.drawEllipse(QPointF(12, 12), 2, 2);
            p.drawArc(QRectF(6, 6, 12, 12), -55 * 16, 110 * 16);
            p.drawArc(QRectF(6, 6, 12, 12), 125 * 16, 110 * 16);
            p.drawArc(QRectF(2, 2, 20, 20), -45 * 16, 90 * 16);
            p.drawArc(QRectF(2, 2, 20, 20), 135 * 16, 90 * 16);
        } else if (name == "preview") {
            p.drawRoundedRect(QRectF(3, 4, 18, 14), 2, 2);
            QPainterPath path;
            path.moveTo(10, 8);
            path.lineTo(16, 11.5);
            path.lineTo(10, 15);
            path.closeSubpath();
            p.drawPath(path);
            p.drawLine(8, 21, 16, 21);
        } else if (name == "play") {
            // E768: il triangolo pieno dell'avvio.
            QPainterPath path;
            path.moveTo(8, 5);
            path.lineTo(19, 12);
            path.lineTo(8, 19);
            path.closeSubpath();
            p.setBrush(inkFill);
            p.setPen(Qt::NoPen);
            p.drawPath(path);
        } else if (name == "stop") {
            // E71A: il quadrato pieno dell'arresto.
            p.setBrush(inkFill);
            p.setPen(Qt::NoPen);
            p.drawRect(QRectF(7, 7, 10, 10));
        } else if (name == "restart") {
            // E72C: freccia circolare del riavvio.
            p.drawArc(QRectF(4, 4, 16, 16), 60 * 16, 280 * 16);
            QPainterPath head;
            head.moveTo(15, 2.5);
            head.lineTo(19, 6.5);
            head.lineTo(14, 7.5);
            head.closeSubpath();
            p.setBrush(inkFill);
            p.setPen(Qt::NoPen);
            p.drawPath(head);
        } else if (name == "edit") {
            QPainterPath pencil;
            pencil.moveTo(5, 15);
            pencil.lineTo(16, 4);
            pencil.lineTo(20, 8);
            pencil.lineTo(9, 19);
            pencil.lineTo(4, 20);
            pencil.closeSubpath();
            p.drawPath(pencil);
            p.drawLine(13, 7, 17, 11);
        } else if (name == "delete") {
            // E74D: il cestino.
            p.drawLine(4, 6, 20, 6);
            p.drawLine(10, 3.5, 14, 3.5);
            QPainterPath body;
            body.moveTo(6, 6);
            body.lineTo(6.8, 20);
            body.lineTo(17.2, 20);
            body.lineTo(18, 6);
            p.drawPath(body);
            p.drawLine(10, 9, 10, 17);
            p.drawLine(14, 9, 14, 17);
        } else if (name == "settings") {
            // E713: ingranaggio.
            for (int i = 0; i < 8; ++i) {
                const double angle = i * pi / 4.0;
                const double inner = 6.4, outer = 9.2;
                p.drawLine(QPointF(12 + std::cos(angle) * inner, 12 + std::sin(angle) * inner),
                           QPointF(12 + std::cos(angle) * outer, 12 + std::sin(angle) * outer));
            }
            p.drawEllipse(QPointF(12, 12), 5.2, 5.2);
        } else if (name == "link") {
            // E71B: due anelli di una catena.
            p.save();
            p.translate(12, 12);
            p.rotate(-45);
            p.drawRoundedRect(QRectF(-7.5, -3, 9, 6), 3, 3);
            p.drawRoundedRect(QRectF(-1.5, -3, 9, 6), 3, 3);
            p.restore();
        } else if (name == "unlock") {
            // E785: lucchetto aperto.
            p.drawArc(QRectF(9, 5, 7, 7), 10 * 16, 150 * 16);
            p.drawRoundedRect(QRectF(5.5, 10.5, 11, 9), 1.8, 1.8);
            p.drawEllipse(QPointF(11, 15), 1.2, 1.2);
        } else if (name == "back" || name == "previousPage") {
            // E72B: chevron a sinistra.
            QPolygonF points;
            points << QPointF(14.5, 5) << QPointF(7.5, 12) << QPointF(14.5, 19);
            p.drawPolyline(points);
        } else if (name == "next" || name == "nextPage") {
            // E72A: chevron a destra.
            QPolygonF points;
            points << QPointF(9.5, 5) << QPointF(16.5, 12) << QPointF(9.5, 19);
            p.drawPolyline(points);
        } else if (name == "close") {
            // E711: la croce.
            p.drawLine(6, 6, 18, 18);
            p.drawLine(18, 6, 6, 18);
        } else if (name == "playlist") {
            // E8FD: la lista con il segno di riproduzione.
            for (int y : {6, 12, 18})
                p.drawLine(11, y, 20, y);
            QPainterPath path;
            path.moveTo(3, 5);
            path.lineTo(9, 9);
            path.lineTo(3, 13);
            path.closeSubpath();
            p.setBrush(inkFill);
            p.setPen(Qt::NoPen);
            p.drawPath(path);
        } else if (name == "deck" || name == "buttons") {
            // Il marchio del deck scene: quattro tasti, tre pieni e uno libero.
            p.setPen(Qt::NoPen);
            p.setBrush(inkFill);
            p.drawRoundedRect(QRectF(2.5, 2.5, 8.6, 8.6), 2.2, 2.2);
            p.drawRoundedRect(QRectF(12.9, 2.5, 8.6, 8.6), 2.2, 2.2);
            p.drawRoundedRect(QRectF(2.5, 12.9, 8.6, 8.6), 2.2, 2.2);
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(ink, 1.6));
            p.drawRoundedRect(QRectF(13.7, 13.7, 7, 7), 1.8, 1.8);
        } else if (name == "spot") {
            // Il marchio di uno spot: il riquadro "Ad" con il segno di riproduzione.
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(QRectF(2.5, 6, 19, 12), 2.5, 2.5);
            QPainterPath path;
            path.moveTo(8, 8.6);
            path.lineTo(15, 12);
            path.lineTo(8, 15.4);
            path.closeSubpath();
            p.setBrush(inkFill);
            p.setPen(Qt::NoPen);
            p.drawPath(path);
        } else if (name == "empty") {
            // E714: contenitore vuoto.
            p.drawRoundedRect(QRectF(3, 5, 18, 14), 2, 2);
            p.drawLine(3, 12, 9, 12);
            p.drawLine(15, 12, 21, 12);
            p.drawArc(QRectF(6.5, 9.5, 5, 5), 180 * 16, 180 * 16);
            p.drawArc(QRectF(12.5, 9.5, 5, 5), 180 * 16, 180 * 16);
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
        } else if (name == "manual") {
            p.drawRoundedRect(QRectF(4, 3, 16, 18), 2, 2);
            for (int y : {8, 12, 16})
                p.drawLine(8, y, 16, y);
        } else if (name == "plus") {
            p.drawLine(5, 12, 19, 12);
            p.drawLine(12, 5, 12, 19);
        } else {
            p.setBrush(QColor("#262628"));
            for (int x : {6, 12, 18}) {
                p.drawLine(x, 4, x, 20);
                p.drawEllipse(QPointF(x, x == 12 ? 15 : 9), 2.5, 2.5);
            }
        }
        p.end();
        icon.addPixmap(pixmap);
    }
    return icon;
}

// I marchi disegnano il path ufficiale della piattaforma: il colore è quello del brand, con il
// fondo bianco del logo di Facebook e le ombre ciano e rosse della nota di TikTok.
QIcon platformMark(const QString& platform) {
    static const QMap<QString, QString> marks = {{"twitch", twitchMark},
                                                 {"youtube", youtubeMark},
                                                 {"kick", kickMark},
                                                 {"facebook", facebookMark},
                                                 {"tiktok", tiktokMark},
                                                 {"rumble", rumbleMark},
                                                 {"x", xMark}};
    static const QMap<QString, QColor> colors = {
        {"twitch", QColor("#bf5af2")}, {"youtube", QColor("#ff453a")},
        {"kick", QColor("#53fc18")},   {"facebook", QColor("#015eec")},
        {"tiktok", QColor("#f5f5f7")}, {"rumble", QColor("#85c742")},
        {"x", QColor("#e7e9ea")}};
    const QPainterPath path = pathFromSvg(marks.value(platform, playMark));
    const QColor color = colors.value(platform, QColor("#98989d"));
    QIcon icon;
    for (int scale : {1, 2, 3}) {
        QPixmap pixmap(24 * scale, 24 * scale);
        pixmap.fill(Qt::transparent);
        pixmap.setDevicePixelRatio(scale);
        QPainter p(&pixmap);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        if (platform == "facebook") {
            // Il ritaglio del logo mostra il bianco che sta dietro al quadrato blu.
            p.setBrush(Qt::white);
            p.drawRect(QRectF(0, 0, 24, 24));
        } else if (platform == "tiktok") {
            p.setBrush(QColor("#25f4ee"));
            p.save();
            p.translate(-0.8, -0.8);
            p.drawPath(path);
            p.restore();
            p.setBrush(QColor("#fe2c55"));
            p.save();
            p.translate(0.8, 0.8);
            p.drawPath(path);
            p.restore();
        }
        p.setBrush(color);
        p.drawPath(path);
        p.end();
        icon.addPixmap(pixmap);
    }
    return icon;
}

// La palette copre anche i controlli nativi non stilizzati e i dialoghi modali.
void applyNativeTheme(const QString& theme) {
    const bool light = theme == "light";
    qApp->setProperty("orbisTheme", light ? "light" : "dark");
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
    if (light) {
        palette.setColor(QPalette::Window, QColor("#f4f4f7"));
        palette.setColor(QPalette::WindowText, QColor("#1d1d1f"));
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::AlternateBase, QColor("#fafafa"));
        palette.setColor(QPalette::Text, QColor("#1d1d1f"));
        palette.setColor(QPalette::Button, Qt::white);
        palette.setColor(QPalette::ButtonText, QColor("#1d1d1f"));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, QColor("#1d1d1f"));
        palette.setColor(QPalette::Link, QColor("#007aff"));
    }
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
        QFrame#layoutRow { background: #262628; border: 1px solid #414145; border-radius: 12px; }
        QFrame#layoutRow:hover { border-color: #58585f; }
        QLabel#layoutLead { background: #007aff; border-radius: 9px; }
        QFrame#layoutEmpty { background: #262628; border: 2px dashed #58585f; border-radius: 12px; }
        QLabel#layoutEmptyIcon { background: #3a3a3e; border-radius: 12px; }
        QLabel#emptyState { background: #232325; border: 1px dashed #58585f; border-radius: 12px; color: #b9b9c0; padding: 24px; }
        QLabel#cardTitle { font-size: 16px; font-weight: 600; }
        QPushButton[card="true"] { border-top: 3px solid #007aff; padding: 0; }
        QPushButton[card="true"] QLabel { background: transparent; border: none; }
        QPushButton[compact="true"] { padding: 6px 10px; }
        QListWidget#wizardProfile, QListWidget#wizardChannel { background: transparent; border: none; }
        QListWidget#wizardProfile::item, QListWidget#wizardChannel::item { background: #323234; border: 1px solid #505055; border-radius: 9px; padding: 14px; margin-bottom: 8px; }
        QListWidget#wizardProfile::item:selected, QListWidget#wizardChannel::item:selected { background: #193f66; border: 2px solid #0a84ff; }
        QListWidget#wizardProfile::item:hover, QListWidget#wizardChannel::item:hover { border-color: #8d8d94; }
        QLabel[badge="live"] { color: #ffffff; background: #0a84ff; border-radius: 9px; padding: 2px 8px; font-size: 10px; font-weight: 600; }
        QLabel[badge="ended"] { color: #30d158; background: #2630d158; border-radius: 9px; padding: 2px 8px; font-size: 10px; font-weight: 600; }
        QLabel[badge="error"] { color: #ff453a; background: #26ff453a; border-radius: 9px; padding: 2px 8px; font-size: 10px; font-weight: 600; }
        QLabel[badge="stopped"] { color: #ffd60a; background: #26ffd60a; border-radius: 9px; padding: 2px 8px; font-size: 10px; font-weight: 600; }
        QLabel[badge="offline"] { color: #98989d; background: #1affffff; border-radius: 9px; padding: 2px 8px; font-size: 10px; font-weight: 600; }
        QLabel[badge="playlist"] { color: #409cff; background: #260a84ff; border-radius: 9px; padding: 2px 8px; font-size: 10px; font-weight: 600; }
        QLabel#rowTitle { font-weight: 600; }
        QLabel#rowCaption { color: #98989d; font-size: 11px; }
        QToolButton[rowAction="true"] { background: transparent; border: none; padding: 4px; border-radius: 6px; }
        QToolButton[rowAction="true"]:hover { background: #1affffff; }
        QToolButton[rowAction="true"]:pressed { background: #33ffffff; }
        QToolButton[rowAction="true"]:disabled { background: transparent; }
        QToolButton[mark="true"] { background: transparent; border: none; padding: 2px; border-radius: 6px; }
        QToolButton[mark="true"]:hover { background: #14ffffff; }
    )");
    // Colori del tema chiaro di tokens.css, sostituiti in un solo passaggio per non
    // trasformare nuovamente i colori già risolti (e mantenere i colori dei marchi).
    if (light) {
        const QMap<QString, QString> colors = {
            {"#1a1a1c", "#f4f4f7"},     {"#232325", "#eeeef1"},    {"#262628", "#ffffff"},
            {"#2b2b2e", "#fafafa"},     {"#323234", "#ffffff"},    {"#3e3e42", "#f5f5f8"},
            {"#414145", "#dedee2"},     {"#505055", "#d5d5da"},    {"#58585f", "#c5c5ca"},
            {"#f5f5f7", "#1d1d1f"},     {"#f5f8ff", "#1d1d1f"},    {"#b2b2ba", "#6e6e73"},
            {"#b9b9c0", "#6e6e73"},     {"#c4c4cc", "#6e6e73"},    {"#ffb0b8", "#c92b23"},
            {"#ff453a", "#ff3b30"},     {"#30d158", "#248a3d"},    {"#ffd60a", "#945800"},
            {"#98989d", "#6e6e73"},     {"#409cff", "#007aff"},    {"#1affffff", "#09000000"},
            {"#33ffffff", "#12000000"}, {"#14ffffff", "#09000000"}};
        const QRegularExpression hex("#[0-9a-fA-F]{6,8}\\b");
        auto matches = hex.globalMatch(stylesheet);
        QString resolved;
        qsizetype offset = 0;
        while (matches.hasNext()) {
            auto match = matches.next();
            resolved += stylesheet.mid(offset, match.capturedStart() - offset);
            resolved += colors.value(match.captured().toLower(), match.captured());
            offset = match.capturedEnd();
        }
        stylesheet = resolved + stylesheet.mid(offset);
        stylesheet += "QToolTip { color: #1d1d1f; background: #ffffff; }";
    }
    // Dashboard: misure e token di base.css, surfaces.css e dashboard.css su Windows.
    stylesheet += QString(R"(
        QWidget#workspace[dashboard="true"] { background: %1; font-size: 13px; }
        QWidget#workspace[dashboard="true"] QFrame#pageHeading { background: %2; border: 1px solid %3; }
        QLabel#pageTitle[dashboardTitle="true"] { font-size: 24px; font-weight: 700; }
        QLabel#dashboardSectionTitle { font-size: 13px; font-weight: 600; }
        QLabel#dashboardSectionAccent { background: %4; border-radius: 1px; }
        QPushButton[dashboardTile="true"] { background: %2; border: 1px solid %3; border-top: 3px solid %4; border-radius: 12px; padding: 0; text-align: left; }
        QPushButton[dashboardTile="true"]:hover { border-color: %5; border-top-color: %4; }
        QPushButton[dashboardTile="true"]:focus { border-color: %4; }
        QPushButton[dashboardTile="true"]:pressed { background: %6; }
        QLabel#dashboardTileIcon { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 %4, stop:1 %7); border-radius: 10px; }
        QLabel#dashboardTileTitle { font-size: 15px; font-weight: 600; background: transparent; }
        QLabel#dashboardTileDescription { font-size: 12px; color: %8; background: transparent; }
        QPushButton#coffeeBanner { border-radius: 16px; min-height: 20px; padding: 6px 14px; font-size: 13px; font-weight: 500; }
        QPushButton#coffeeBanner:hover { border-color: #99ff9500; color: #e07b00; }
        QLabel#coffeePerla { color: %8; font-size: 11px; }
        QWidget#themeSelector { background: %6; border: 1px solid %3; border-radius: 8px; }
        QToolButton[themeChoice="true"] { background: transparent; border: none; padding: 6px; }
        QToolButton[themeChoice="true"]:checked { background: %2; border: 1px solid %5; }
    )")
                      .arg(light ? "#f4f4f7" : "#1a1a1c", light ? "#ffffff" : "#262628",
                           light ? "#0f000000" : "#14ffffff", light ? "#007aff" : "#0a84ff",
                           light ? "#1f000000" : "#29ffffff", light ? "#f5f5f8" : "#323234",
                           light ? "#0068d9" : "#0970d9", light ? "#6e6e73" : "#98989d");
    stylesheet += QString(R"(
        QWidget#sidebar { background: %1; border: 1px solid %2; border-radius: 12px; }
        QWidget#sidebar[closed="true"] { background: transparent; border: none; }
        QLabel#sidebarBrand { font-size: 14px; font-weight: 600; }
        QLabel#sidebarLogo { background: %3; border-radius: 10px; }
        QToolButton#sidebarToggle { border: none; background: transparent; padding: 0; border-radius: 6px; }
        QToolButton#sidebarToggle:hover { background: %2; }
        QListWidget#navigation { border: none; padding: 0; margin: 0; font-size: 13px; }
        QListWidget#navigation::item { height: 32px; padding: 0 10px; margin: 0 0 3px; border-radius: 6px; color: %4; }
        QListWidget#navigation::item:selected { background: %3; color: white; font-weight: 600; }
        QListWidget#navigation::item:hover:!selected { background: %2; }
        QWidget#sidebar[compact="true"] QListWidget#navigation::item { padding: 0 10px; }
        QComboBox#languagePicker { background: transparent; border: none; padding: 0 10px; font-size: 13px; color: %4; }
        QComboBox#languagePicker:hover { background: %2; }
        QComboBox#languagePicker[compact="true"] { padding: 0 5px; }
        QComboBox#languagePicker[compact="true"]::drop-down { width: 0; }
        QComboBox#languagePicker QAbstractItemView { min-width: 185px; }
        QPushButton#sidebarAbout { background: transparent; border: none; border-top: 1px solid %2; border-radius: 6px; padding: 0; }
        QPushButton#sidebarAbout:hover { background: %2; }
        QLabel#sidebarVersion { font-size: 11px; font-weight: 600; color: %4; }
        QLabel#sidebarAuthor { font-size: 10px; color: %4; }
    )")
                      .arg(light ? "#ffffff" : "#262628", light ? "#0f000000" : "#14ffffff",
                           light ? "#007aff" : "#0a84ff", light ? "#6e6e73" : "#98989d");
    qApp->setStyleSheet(stylesheet);
}
} // namespace orbis

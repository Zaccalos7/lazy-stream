#pragma once
#include <QColor>
#include <QIcon>
#include <QString>

namespace orbis {
// Stile e icone disegnati da Qt: nessuna risorsa HTML o dipendenza dal tema del desktop.
void applyNativeTheme(const QString& theme = "dark");
// Glifi vettoriali nella forma di quelli di Windows (Segoe MDL2): il colore predefinito è il testo
// secondario del tema, ma le azioni delle righe lo cambiano (pericolo, successo, attenzione).
QIcon studioIcon(const QString& name, const QColor& color = QColor());
// Marchio di una piattaforma, disegnato dal path SVG ufficiale nel colore del brand.
QIcon platformMark(const QString& platform);
} // namespace orbis

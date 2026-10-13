#pragma once
#include <QIcon>
#include <QString>

namespace orbis {
// Stile e icone disegnati da Qt: nessuna risorsa HTML o dipendenza dal tema del desktop.
void applyNativeTheme();
QIcon studioIcon(const QString& name);
} // namespace orbis

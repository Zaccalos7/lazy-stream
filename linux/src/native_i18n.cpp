#include "native_i18n.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QLocale>
#include <stdexcept>

namespace orbis {
// L'assenza dei cataloghi è un errore di installazione, non una finestra con etichette incomplete.
NativeTranslations::NativeTranslations(QString directory) : directory(std::move(directory)) {
    if (!languages().contains("en")) throw std::runtime_error("Translation catalogs missing");
    setLanguage("en");
}

// Scopre tutte le lingue installate: aggiungere un catalogo non richiede ricompilare la UI.
QStringList NativeTranslations::languages() const {
    QStringList result;
    for (const auto& file : QDir(directory).entryList({"ui.*.json"}, QDir::Files, QDir::Name))
        result.append(file.mid(3, file.size() - 8));
    return result;
}

// Applica lingua, formati numerici e direzione nello stesso punto per evitare pannelli RTL incoerenti.
void NativeTranslations::setLanguage(const QString& language) {
    QFile file(directory + "/ui." + language + ".json");
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Translation catalog unavailable");
    QJsonParseError error;
    auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        throw std::runtime_error("Invalid translation catalog");
    catalog = document.object();
    selected = language;
    QLocale::setDefault(QLocale(language));
    if (qApp) qApp->setLayoutDirection(language == "ar" ? Qt::RightToLeft : Qt::LeftToRight);
}

// Tutti i testi della UI sono chiavi originali; i test controllano che esistano in ogni catalogo.
QString NativeTranslations::text(const char* key) const {
    const auto value = catalog.value(QString::fromUtf8(key));
    if (!value.isString()) throw std::runtime_error(std::string("Missing translation: ") + key);
    return value.toString();
}

QStringList NativeTranslations::keys() const { return catalog.keys(); }

// Traduce anche i pulsanti standard Qt con le parole del catalogo, comprese le lingue non fornite da Qt.
QString NativeTranslations::translate(const char*, const char* source, const char*, int) const {
    const QString value = QString::fromUtf8(source);
    const QMap<QString, const char*> standard = {
        {"Cancel", "cancel"}, {"&Cancel", "cancel"}, {"Save", "save"}, {"&Save", "save"},
        {"Close", "close"}, {"&Close", "close"}, {"Open", "sceneMediaChoose"}, {"&Open", "sceneMediaChoose"},
        {"Yes", "confirm"}, {"&Yes", "confirm"}, {"No", "cancel"}, {"&No", "cancel"}, {"OK", "confirm"},
        {"Back", "back"}, {"&Back", "back"}, {"Next", "next"}, {"&Next", "next"}, {"Finish", "startLiveButton"},
        {"File name:", "sceneButtonName"}, {"Files of type:", "sceneMedia"}, {"Look in:", "videoFolder"},
        {"File &name:", "sceneButtonName"}, {"Files of &type:", "sceneMedia"}, {"Look &in:", "videoFolder"}
    };
    auto found = standard.find(value);
    return found == standard.end() ? QString{} : text(found.value());
}

// Gli autonimi rendono riconoscibile la propria lingua anche quando l'interfaccia ne usa un'altra.
QString NativeTranslations::languageName(const QString& code) {
    const QMap<QString, QString> names = {{"fil", "Filipino"}, {"la", "Latina"}, {"tlh", "tlhIngan Hol"}, {"hod", "Hodor"}, {"zh", "中文"}, {"ar", "العربية"}};
    if (names.contains(code)) return names.value(code);
    auto name = QLocale(code).nativeLanguageName();
    return name.isEmpty() ? code : name;
}
} // namespace orbis

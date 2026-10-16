#pragma once
#include <QJsonObject>
#include <QStringList>
#include <QTranslator>

namespace orbis {
// Legge i cataloghi condivisi delle lingue, adattati alle piattaforme supportate su Linux.
class NativeTranslations : public QTranslator {
    QString directory, selected;
    QJsonObject catalog;
    QTranslator qtDialogs;

  public:
    explicit NativeTranslations(QString directory);
    QStringList languages() const;
    void setLanguage(const QString& language);

    QString language() const {
        return selected;
    }

    QString text(const char* key) const;
    QStringList keys() const;

    bool isEmpty() const override {
        return catalog.isEmpty();
    }

    QString translate(const char* context, const char* source, const char* disambiguation = nullptr,
                      int n = -1) const override;
    static QString languageName(const QString& code);
};
} // namespace orbis

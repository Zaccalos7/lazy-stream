#pragma once
#include "core.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace orbis {
// Moduli nativi tipizzati: i campi non mostrati restano nel JSON, evitando di perdere opzioni esistenti.
class NativeForm : public QDialog {
    Json original;
    QMap<QString, QWidget*> fields;
  public:
    QVBoxLayout* layout;
    QFormLayout* form;
    QDialogButtonBox* buttons;
    explicit NativeForm(QWidget* parent, QString title, Json original = Json{}) : QDialog(parent), original(original.clone()) {
        setWindowTitle(title);
        setMinimumWidth(480);
        layout = new QVBoxLayout(this);
        form = new QFormLayout;
        layout->addLayout(form);
        buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }
    // Il nome del widget è il campo dati, così i test possono interagire in tutte le lingue.
    QLineEdit* text(const char* key, const QString& label, bool password = false) {
        auto input = new QLineEdit(QString::fromStdString(original.get(key).str()), this);
        input->setObjectName(QString::fromUtf8(key));
        if (password) input->setEchoMode(QLineEdit::Password);
        fields[key] = input;
        form->addRow(label, input);
        return input;
    }
    QDoubleSpinBox* number(const char* key, const QString& label, double fallback, double low, double high, int decimals = 0) {
        auto input = new QDoubleSpinBox(this);
        input->setObjectName(key);
        input->setDecimals(decimals);
        input->setRange(low, high);
        input->setValue(original.get(key).num(fallback));
        fields[key] = input;
        form->addRow(label, input);
        return input;
    }
    QCheckBox* check(const char* key, const QString& label, bool fallback = false) {
        auto input = new QCheckBox(label, this);
        input->setObjectName(key);
        input->setChecked(original.get(key).flag(fallback));
        fields[key] = input;
        form->addRow(input);
        return input;
    }
    QComboBox* choice(const char* key, const QString& label, const QList<QPair<QString, Json>>& values) {
        auto input = new QComboBox(this);
        input->setObjectName(key);
        for (const auto& entry : values) input->addItem(entry.first, QString::fromStdString(entry.second.dump()));
        const auto selected = input->findData(QString::fromStdString(original.get(key).dump()));
        if (selected >= 0) input->setCurrentIndex(selected);
        fields[key] = input;
        form->addRow(label, input);
        return input;
    }
    // Legge i valori come numeri/booleani reali, non come stringhe di un form web.
    Json value() const {
        auto result = original.clone();
        for (auto it = fields.begin(); it != fields.end(); ++it) {
            const auto key = it.key().toStdString();
            if (auto input = qobject_cast<QLineEdit*>(it.value())) result.set(key, input->text().toStdString());
            else if (auto input = qobject_cast<QDoubleSpinBox*>(it.value())) result.set(key, input->value());
            else if (auto input = qobject_cast<QCheckBox*>(it.value())) result.set(key, input->isChecked());
            else if (auto input = qobject_cast<QComboBox*>(it.value())) result.set(key, Json::parse(input->currentData().toString().toStdString()));
        }
        return result;
    }
};
} // namespace orbis

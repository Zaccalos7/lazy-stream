#pragma once
#include "core.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

namespace orbis {
// Moduli nativi tipizzati: i campi non mostrati restano nel JSON, evitando di perdere opzioni
// esistenti.
class NativeForm : public QDialog {
    Json original;
    QMap<QString, QWidget*> fields;

  public:
    QVBoxLayout* layout;
    QFormLayout* form;
    QDialogButtonBox* buttons;

    explicit NativeForm(QWidget* parent, QString title, Json original = Json{})
        : QDialog(parent), original(original.clone()) {
        setWindowTitle(title);
        setMinimumWidth(560);
        layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(18);
        auto heading = new QLabel(title, this);
        heading->setObjectName("heroTitle");
        heading->setWordWrap(true);
        layout->addWidget(heading);
        auto scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        auto fieldsContainer = new QWidget(scroll);
        form = new QFormLayout(fieldsContainer);
        form->setContentsMargins(0, 0, 10, 0);
        form->setHorizontalSpacing(20);
        form->setVerticalSpacing(10);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        form->setRowWrapPolicy(QFormLayout::WrapLongRows);
        scroll->setWidget(fieldsContainer);
        layout->addWidget(scroll, 1);
        resize(680, qMin(800, screen()->availableGeometry().height() - 80));
        buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    // Il nome del widget è il campo dati, così i test possono interagire in tutte le lingue.
    QLineEdit* text(const char* key, const QString& label, bool password = false) {
        auto input = new QLineEdit(QString::fromStdString(original.get(key).str()), this);
        input->setObjectName(QString::fromUtf8(key));
        if (password)
            input->setEchoMode(QLineEdit::Password);
        fields[key] = input;
        form->addRow(label, input);
        return input;
    }

    // I limiti impediscono valori che il motore non può codificare e mantengono numeri nel
    // database.
    QDoubleSpinBox* number(const char* key, const QString& label, double fallback, double low,
                           double high, int decimals = 0) {
        auto input = new QDoubleSpinBox(this);
        input->setObjectName(key);
        input->setDecimals(decimals);
        input->setRange(low, high);
        input->setValue(original.get(key).num(fallback));
        fields[key] = input;
        form->addRow(label, input);
        return input;
    }

    // Il valore iniziale distingue esplicitamente configurazioni nuove e preferenze già salvate.
    QCheckBox* check(const char* key, const QString& label, bool fallback = false) {
        auto input = new QCheckBox(label, this);
        input->setObjectName(key);
        input->setChecked(original.get(key).flag(fallback));
        fields[key] = input;
        form->addRow(input);
        return input;
    }

    // Il dato JSON è separato dalla dicitura visibile per rendere indipendente la scelta dalla
    // lingua.
    QComboBox* choice(const char* key, const QString& label,
                      const QList<QPair<QString, Json>>& values) {
        auto input = new QComboBox(this);
        input->setObjectName(key);
        for (const auto& entry : values)
            input->addItem(entry.first, QString::fromStdString(entry.second.dump()));
        const auto selected = input->findData(QString::fromStdString(original.get(key).dump()));
        if (selected >= 0)
            input->setCurrentIndex(selected);
        fields[key] = input;
        form->addRow(label, input);
        return input;
    }

    // Legge i valori come numeri/booleani reali, non come stringhe di un form web.
    Json value() const {
        auto result = original.clone();
        for (auto it = fields.begin(); it != fields.end(); ++it) {
            const auto key = it.key().toStdString();
            if (auto input = qobject_cast<QLineEdit*>(it.value()))
                result.set(key, input->text().toStdString());
            else if (auto input = qobject_cast<QDoubleSpinBox*>(it.value()))
                result.set(key, input->value());
            else if (auto input = qobject_cast<QCheckBox*>(it.value()))
                result.set(key, input->isChecked());
            else if (auto input = qobject_cast<QComboBox*>(it.value()))
                result.set(key, Json::parse(input->currentData().toString().toStdString()));
        }
        return result;
    }
};
} // namespace orbis

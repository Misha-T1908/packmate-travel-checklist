#include "AddEditItemDialog.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QVBoxLayout>

AddEditItemDialog::AddEditItemDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Річ для пакування");
    setModal(true);
    resize(420, 320);

    nameEdit = new QLineEdit(this);

    categoryCombo = new QComboBox(this);
    categoryCombo->addItems(categoryLabels());

    quantitySpin = new QSpinBox(this);
    quantitySpin->setRange(1, 999);
    quantitySpin->setValue(1);

    priorityCombo = new QComboBox(this);
    priorityCombo->addItems(priorityLabels());

    noteEdit = new QPlainTextEdit(this);
    noteEdit->setPlaceholderText("Необов'язкова нотатка");
    noteEdit->setFixedHeight(80);

    packedCheck = new QCheckBox("Зібрано", this);

    auto *form = new QFormLayout;
    form->addRow("Назва", nameEdit);
    form->addRow("Категорія", categoryCombo);
    form->addRow("Кількість", quantitySpin);
    form->addRow("Важливість", priorityCombo);
    form->addRow("Нотатка", noteEdit);
    form->addRow("", packedCheck);

    auto *buttons = new QDialogButtonBox(this);
    buttons->addButton("Зберегти", QDialogButtonBox::AcceptRole);
    buttons->addButton("Скасувати", QDialogButtonBox::RejectRole);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox message(this);
            message.setWindowTitle("PackMate");
            message.setIcon(QMessageBox::Warning);
            message.setText("Вкажіть назву речі.");
            message.addButton("Гаразд", QMessageBox::AcceptRole);
            message.exec();
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void AddEditItemDialog::setItem(const PackingItem &item)
{
    currentItem = item;
    nameEdit->setText(item.name);
    categoryCombo->setCurrentText(categoryDisplayString(item.category));
    quantitySpin->setValue(item.quantity);
    priorityCombo->setCurrentText(priorityDisplayString(item.priority));
    noteEdit->setPlainText(item.note);
    packedCheck->setChecked(item.isPacked);
}

PackingItem AddEditItemDialog::item() const
{
    PackingItem updated = currentItem;
    updated.name = nameEdit->text().trimmed();
    updated.category = categoryFromString(categoryCombo->currentText());
    updated.quantity = quantitySpin->value();
    updated.priority = priorityFromString(priorityCombo->currentText());
    updated.note = noteEdit->toPlainText().trimmed();
    updated.isPacked = packedCheck->isChecked();
    return updated;
}

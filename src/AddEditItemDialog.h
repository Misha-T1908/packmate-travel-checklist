#pragma once

#include "Models.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;

class AddEditItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEditItemDialog(QWidget *parent = nullptr);

    void setItem(const PackingItem &item);
    PackingItem item() const;

private:
    QLineEdit *nameEdit = nullptr;
    QComboBox *categoryCombo = nullptr;
    QSpinBox *quantitySpin = nullptr;
    QComboBox *priorityCombo = nullptr;
    QPlainTextEdit *noteEdit = nullptr;
    QCheckBox *packedCheck = nullptr;
    PackingItem currentItem;
};

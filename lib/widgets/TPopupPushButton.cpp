#include "TPopupPushButton.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QObject>
#include <QPushButton>
WARNINGS_ENABLE

TPopupPushButton::TPopupPushButton(QWidget *parent)
    : QPushButton(parent), _popup(nullptr)
{
    setCheckable(true);
    connect(this, &QPushButton::toggled, this, &TPopupPushButton::valueChanged);
}

void TPopupPushButton::setPopup(QDialog *popup)
{
    _popup = popup;
    connect(_popup, &QDialog::finished, this, &TPopupPushButton::popupClosed);
}

void TPopupPushButton::valueChanged(bool checked)
{
    if(_popup != nullptr)
        _popup->setVisible(checked);
}

void TPopupPushButton::popupClosed(int result)
{
    Q_UNUSED(result)
    setChecked(false);
}

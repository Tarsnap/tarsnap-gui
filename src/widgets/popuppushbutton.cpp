#include "popuppushbutton.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QObject>
#include <QPushButton>
WARNINGS_ENABLE

PopupPushButton::PopupPushButton(QWidget *parent)
    : QPushButton(parent), _popup(nullptr)
{
    connect(this, &QPushButton::toggled, this, &PopupPushButton::valueChanged);
}

PopupPushButton::~PopupPushButton()
{
}

void PopupPushButton::setPopup(QDialog *popup)
{
    _popup = popup;
    connect(_popup, &QDialog::finished, this, &PopupPushButton::popupClosed);
}

void PopupPushButton::valueChanged(bool checked)
{
    if(_popup != nullptr)
    {
        _popup->setVisible(checked);
    }
}

void PopupPushButton::popupClosed(int result)
{
    Q_UNUSED(result)
    setChecked(false);
}

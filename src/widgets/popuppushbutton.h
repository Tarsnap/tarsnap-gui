#ifndef POPUPPUSHBUTTON_H
#define POPUPPUSHBUTTON_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QPushButton>
WARNINGS_ENABLE

/*!
 * \ingroup widgets-general
 * \brief The PopupPushButton widget is a QPushButton which shows a non-modal
 * QDialog in a pop-up window.
 */
class PopupPushButton : public QPushButton
{
    Q_OBJECT

public:
    //! Constructor.
    explicit PopupPushButton(QWidget *parent = nullptr);
    ~PopupPushButton();

    //! Display popup if and only if this button is checked.
    void setPopup(QDialog *popup);

private slots:
    void valueChanged(bool checked);
    void popupClosed(int result);

private:
    QDialog *_popup;
};

#endif // POPUPPUSHBUTTON_H

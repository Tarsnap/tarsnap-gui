#ifndef TPOPUPPUSHBUTTON_H
#define TPOPUPPUSHBUTTON_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QPushButton>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QDialog;
class QWidget;

/*!
 * \ingroup widgets-general
 * \brief The TPopupPushButton is a QPushButton which shows a non-modal
 * QDialog in a pop-up window.
 */
class TPopupPushButton : public QPushButton
{
    Q_OBJECT

public:
    //! Constructor.
    explicit TPopupPushButton(QWidget *parent = nullptr);

    //! Display popup if and only if this button is checked.
    void setPopup(QDialog *popup);

private slots:
    void valueChanged(bool checked);
    void popupClosed(int result);

private:
    QDialog *_popup;
};

#endif /* !TPOPUPPUSHBUTTON_H */

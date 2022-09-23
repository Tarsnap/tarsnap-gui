#include "TOkLabel.h"

WARNINGS_DISABLE
#include <QEvent>
#include <Qt>
WARNINGS_ENABLE

TOkLabel::TOkLabel(QWidget *parent) : QLabel(parent), _status(Unset)
{
    setAlignment(Qt::AlignCenter);
    setFixedSize(15, 15);
}

void TOkLabel::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
        setStatus(_status);
    else
        QLabel::changeEvent(event);
}

TOkLabel::Status TOkLabel::status() const
{
    return (_status);
}

void TOkLabel::setStatus(Status status)
{
    _status = status;

    switch(status)
    {
    case Unset:
        setText("");
        setStyleSheet("");
        break;
    case Ok:
        setText(tr("✔"));
        setStyleSheet("color: green");
        break;
    case Error:
        setText(tr("❌"));
        setStyleSheet("color: darkred");
        break;
    }
}

QString TOkLabel::getRichText(Status status)
{
    switch(status)
    {
    case Ok:
        return ("<font color=\"green\">" + tr("✔") + "</font>");
    case Error:
        return ("<font color=\"darkred\">" + tr("❌") + "</font>");
    case Unset:
        /* FALLTHROUGH */
        ;
    }
    return ("");
}

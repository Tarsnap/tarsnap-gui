#include "TOkLabel.h"

WARNINGS_DISABLE
#include <QEvent>
#include <QWidget>
#include <Qt>
WARNINGS_ENABLE

TOkLabel::TOkLabel(QWidget *parent) : QLabel(parent), _status(0)
{
    setAlignment(Qt::AlignCenter);
    setFixedSize(15, 15);
}

void TOkLabel::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
        setStatus(_status);
    else
        QWidget::changeEvent(event);
}

int TOkLabel::status() const
{
    return _status;
}

void TOkLabel::setStatus(int status)
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

const QString TOkLabel::getRichText(int status)
{
    switch(status)
    {
    case Unset:
        return "";
    case Ok:
        return "<font color=\"green\">" + tr("✔") + "</font>";
    case Error:
        return "<font color=\"darkred\">" + tr("❌") + "</font>";
    }
    return "";
}

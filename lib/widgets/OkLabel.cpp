#include "OkLabel.h"

OkLabel::OkLabel(QWidget *parent) : QLabel(parent), _status(0)
{
    setAlignment(Qt::AlignCenter);
    setFixedSize(15, 15);
}

int OkLabel::status() const
{
    return _status;
}

void OkLabel::setStatus(int status)
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

#include "OkLabel.h"

OkLabel::OkLabel(QWidget *parent) : QLabel(parent), _ok(0)
{
    setAlignment(Qt::AlignCenter);
    setFixedSize(15, 15);
}

int OkLabel::status() const
{
    return _ok;
}

void OkLabel::setStatus(int ok)
{
    _ok = ok;

    switch(ok)
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

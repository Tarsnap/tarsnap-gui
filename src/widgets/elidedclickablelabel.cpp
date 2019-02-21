#include "elidedclickablelabel.h"

#include <QMouseEvent>

ElidedClickableLabel::ElidedClickableLabel(QWidget *parent)
    : ElidedLabel(parent)
{
}

ElidedClickableLabel::~ElidedClickableLabel()
{
}

void ElidedClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
    event->accept();
}

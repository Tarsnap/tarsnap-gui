#include "elidedclickablelabel.h"

WARNINGS_DISABLE
#include <QMouseEvent>
WARNINGS_ENABLE

ElidedClickableLabel::ElidedClickableLabel(QWidget *parent)
    : TElidedLabel(parent)
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

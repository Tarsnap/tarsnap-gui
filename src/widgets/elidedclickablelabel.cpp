#include "elidedclickablelabel.h"

WARNINGS_DISABLE
#include <QMouseEvent>
#include <Qt>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QWidget;

ElidedClickableLabel::ElidedClickableLabel(QWidget *parent)
    : TElidedLabel(parent)
{
}

void ElidedClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
    event->accept();
}

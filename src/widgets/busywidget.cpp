#include "busywidget.h"

WARNINGS_DISABLE
#include <QMouseEvent>
WARNINGS_ENABLE

BusyWidget::BusyWidget(QWidget *parent)
    : QLabel(parent), _animation(":/icons/loading.gif")
{
    setMovie(&_animation);
}

BusyWidget::~BusyWidget()
{
}

void BusyWidget::animate()
{
    _animation.start();
    show();
}

void BusyWidget::stop()
{
    hide();
    _animation.stop();
}

void BusyWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
    event->accept();
}

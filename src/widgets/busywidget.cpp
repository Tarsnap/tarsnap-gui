#include "busywidget.h"

#include "debug.h"

BusyWidget::BusyWidget(QWidget *parent):
    QLabel(parent),
    _animation(":/resources/icons/loading.gif")
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
    if (event->button() == Qt::LeftButton)
        emit clicked();
}


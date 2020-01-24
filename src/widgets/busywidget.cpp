#include "busywidget.h"

WARNINGS_DISABLE
#include <QMouseEvent>
#include <QMovie>
WARNINGS_ENABLE

BusyWidget::BusyWidget(QWidget *parent) : QLabel(parent)
{
    // Load the animation.
    _animation = new QMovie(":/icons/loading.gif");
    _animation->setCacheMode(QMovie::CacheAll);
    _animation->jumpToFrame(0);

    // Ensure that this label has the right size.
    const QSize size = _animation->frameRect().size();
    setMinimumSize(size);
    setMaximumSize(size);
}

BusyWidget::~BusyWidget()
{
    delete _animation;
}

void BusyWidget::animate(bool active)
{
    if(active)
    {
        setMovie(_animation);
        _animation->start();
    }
    else
    {
        _animation->stop();
        clear();
    }
}

void BusyWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
    event->accept();
}

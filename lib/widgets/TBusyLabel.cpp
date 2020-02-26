#include "TBusyLabel.h"

WARNINGS_DISABLE
#include <QMouseEvent>
#include <QMovie>
WARNINGS_ENABLE

TBusyLabel::TBusyLabel(QWidget *parent) : QLabel(parent)
{
    // Load the animation.
    _animation = new QMovie(":/lib/loading.gif");
    _animation->setCacheMode(QMovie::CacheAll);
    _animation->jumpToFrame(0);

    // Ensure that this label has the right size.
    const QSize size = _animation->frameRect().size();
    setMinimumSize(size);
    setMaximumSize(size);
}

TBusyLabel::~TBusyLabel()
{
    delete _animation;
}

void TBusyLabel::animate(bool active)
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

void TBusyLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
    event->accept();
}
#include "TBusyLabel.h"

WARNINGS_DISABLE
#include <QFileInfo>
#include <QMouseEvent>
#include <QMovie>
#include <Qt>
WARNINGS_ENABLE

#define MOVIE_FILENAME ":/lib/loading.gif"

TBusyLabel::TBusyLabel(QWidget *parent) : QLabel(parent)
{
    Q_ASSERT(QFileInfo::exists(MOVIE_FILENAME));

    // Load the animation.
    _animation = new QMovie(MOVIE_FILENAME);
    _animation->setCacheMode(QMovie::CacheAll);
    _animation->jumpToFrame(0);

    // Ensure that this label has the right size.
    setFixedSize(16, 16);
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

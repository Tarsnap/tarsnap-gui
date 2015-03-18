#include "joblistitem.h"

JobListItem::JobListItem(QObject *parent) : QObject(parent)
{
    _defaultHeight = sizeHint().height();
    _expandedHeight = EXPANDED_HEIGHT;
    _itemUi.setupUi(&_widget);
//    _widget.installEventFilter(this);
}

JobListItem::~JobListItem()
{

}

QWidget *JobListItem::widget()
{
    return &_widget;
}

void JobListItem::displayDrawer(bool display)
{
    if(display && (_widget.layout()->itemAt(1) == 0))
    {
        // add job widget drawer
        QWidget *jobWidget = new QWidget();
        _jobUi.setupUi(jobWidget);
        connect(_jobUi.browseButton, &QPushButton::clicked,
                [=](){
                    _jobUi.stackedWidget->setCurrentWidget(_jobUi.restoreView);
                });
        connect(_jobUi.backButton, &QPushButton::clicked,
                [=](){
                    _jobUi.stackedWidget->setCurrentWidget(_jobUi.treeView);
                });
        _widget.layout()->addWidget(jobWidget);
        setSizeHint(QSize(sizeHint().width(), _expandedHeight));
    }
    else if (!display && (_widget.layout()->itemAt(1) != 0))
    {
        // remove job widget drawer
        QWidget *jobWidget = _widget.layout()->itemAt(1)->widget();
        _widget.layout()->removeWidget(jobWidget);
        delete jobWidget;
        setSizeHint(QSize(sizeHint().width(), _defaultHeight));
    }
}

void JobListItem::toggleDrawer()
{
    bool display = (_widget.layout()->itemAt(1) == 0);
    displayDrawer(display);
}

//bool JobListItem::eventFilter(QObject *obj, QEvent *event)
//{
//    if (event->type() == QEvent::MouseButtonDblClick) {
//        return true;
//    } else {
//        // standard event processing
//        return QObject::eventFilter(obj, event);
//    }
//}

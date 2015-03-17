#include "jobslistwidget.h"
#include "joblistitem.h"

JobsListWidget::JobsListWidget(QWidget *parent) : QListWidget(parent)
{
    // TODO: REMOVE
    for(int i = 0; i < 50; i++)
    {
        JobListItem *item = new JobListItem();
        insertItem(count(), item);
        setItemWidget(item, item->widget());
    }
    connect(this, &QListWidget::currentItemChanged,
            [=](QListWidgetItem *current, QListWidgetItem *previous)
                {
                    Q_UNUSED(current)
                    QMetaObject::invokeMethod(static_cast<JobListItem*>(previous), "displayDrawer"
                                              , Qt::QueuedConnection, Q_ARG(bool
                                              , false));
                });
    connect(this, &QListWidget::itemActivated,
            [=](QListWidgetItem *item)
                {
                    QMetaObject::invokeMethod(static_cast<JobListItem*>(item), "toggleDrawer"
                                             , Qt::QueuedConnection);
                });
}

JobsListWidget::~JobsListWidget()
{
    clear();
}



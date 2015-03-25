#include "jobslistwidget.h"
#include "joblistitem.h"

JobsListWidget::JobsListWidget(QWidget *parent) : QListWidget(parent)
{
    // TODO: REMOVE
    for(int i = 0; i < 50; i++)
    {
        JobPtr job(new Job);
        job->setUuid(QUuid::createUuid());
        job->setName(QString::number(i).prepend("Backup "));
        JobListItem *item = new JobListItem(job);
        insertItem(count(), item);
        setItemWidget(item, item->widget());
    }
    connect(this, &QListWidget::itemActivated,
            [=](QListWidgetItem *item)
                {
                    emit displayJobDetails(static_cast<JobListItem*>(item)->job());
                });
}

JobsListWidget::~JobsListWidget()
{
    clear();
}



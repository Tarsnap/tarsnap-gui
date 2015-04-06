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
        connect(item, SIGNAL(requestBackup()), this, SLOT(backupItem()));
        connect(item, SIGNAL(requestInspect()), this, SLOT(inspectItem()));
        connect(item, SIGNAL(requestRestore()), this, SLOT(restoreItem()));
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

void JobsListWidget::backupItem()
{

}

void JobsListWidget::inspectItem()
{
    if(sender())
    {
        emit displayJobDetails(qobject_cast<JobListItem*>(sender())->job());
    }
}

void JobsListWidget::restoreItem()
{

}



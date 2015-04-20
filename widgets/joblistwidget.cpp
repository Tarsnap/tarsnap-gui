#include "joblistwidget.h"
#include "joblistitem.h"
#include "debug.h"

JobListWidget::JobListWidget(QWidget *parent) : QListWidget(parent)
{
    reloadJobs();
    connect(this, &QListWidget::itemActivated,
            [=](QListWidgetItem *item)
                {
                    emit displayJobDetails(static_cast<JobListItem*>(item)->job());
            });
}

JobListWidget::~JobListWidget()
{
    clear();
}

void JobListWidget::backupItem()
{

}

void JobListWidget::inspectItem()
{
    if(sender())
    {
        emit displayJobDetails(qobject_cast<JobListItem*>(sender())->job());
    }
}

void JobListWidget::restoreItem()
{

}

void JobListWidget::reloadJobs()
{
    clear();
    foreach(JobPtr job, getStoredJobs())
    {
        addJob(job);
    }
}

void JobListWidget::addJob(JobPtr job)
{
    if(job)
    {
        JobListItem *item = new JobListItem(job);
        connect(item, SIGNAL(requestBackup()), this, SLOT(backupItem()));
        connect(item, SIGNAL(requestInspect()), this, SLOT(inspectItem()));
        connect(item, SIGNAL(requestRestore()), this, SLOT(restoreItem()));
        insertItem(count(), item);
        setItemWidget(item, item->widget());
    }
}

QList<JobPtr> JobListWidget::getStoredJobs()
{
    QList<JobPtr> jobs;
    QSqlQuery query;
    if(!query.prepare(QLatin1String("select name from jobs")))
    {
        DEBUG << query.lastError().text();
        return jobs;
    }
    PersistentStore& store = PersistentStore::instance();
    if(!store.initialized())
    {
        DEBUG << "PersistentStore was not initialized.";
        return jobs;
    }
    if(!query.exec())
    {
        DEBUG << query.lastError().text();
    }
    else if(query.next())
    {
        do
        {
            JobPtr job(new Job);
            job->setName(query.value(query.record().indexOf("name")).toString());
            job->load();
            jobs << job;
        }while(query.next());
    }
    return jobs;
}



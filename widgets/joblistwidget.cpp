#include "joblistwidget.h"
#include "joblistitem.h"
#include "debug.h"

#include <QMessageBox>

#define JOB_NAME_PREFIX    QLatin1String("Job")
#define JOB_NAME_SEPARATOR QLatin1String("_")

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
    if(sender())
    {
        JobPtr job = qobject_cast<JobListItem*>(sender())->job();
        if(job)
        {
            BackupTaskPtr backup(new BackupTask);
            backup->setName(JOB_NAME_PREFIX + JOB_NAME_SEPARATOR + job->name() + JOB_NAME_SEPARATOR + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH:mm:ss"));
            backup->setUrls(job->urls());
            connect(backup, SIGNAL(statusUpdate()), this, SLOT(backupTaskUpdate()), Qt::QueuedConnection);
            emit backupJob(backup);
        }
    }
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

void JobListWidget::deleteItem()
{
    JobListItem* jobItem = qobject_cast<JobListItem*>(sender());
    if(jobItem)
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Confirm action")
                                                                   , tr("Are you sure you want to delete job %1 (this cannot be undone)?").arg(jobItem->job()->name()));
        if(button == QMessageBox::Yes)
        {
            jobItem->job()->purge();
            QListWidgetItem *item = this->takeItem(this->row(jobItem));
            if(item)
                delete item;
        }
    }
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
        connect(item, SIGNAL(requestDelete()), this, SLOT(deleteItem()));
        insertItem(count(), item);
        setItemWidget(item, item->widget());
    }
}

void JobListWidget::backupTaskUpdate()
{

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



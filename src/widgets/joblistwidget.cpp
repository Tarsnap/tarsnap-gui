#include "joblistwidget.h"
#include "joblistitem.h"
#include "restoredialog.h"
#include "debug.h"

#include <QMessageBox>

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

void JobListWidget::backupSelectedItems()
{
    if(selectedItems().isEmpty())
        return;

    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Confirm action")
                                                               , tr("Initiate backup for the %1 selected job(s)?").arg(selectedItems().count()));
    if(button == QMessageBox::Yes)
    {
        foreach (QListWidgetItem *item, this->selectedItems())
        {
            if(item->isSelected())
            {
                JobPtr job = static_cast<JobListItem*>(item)->job();
                emit backupJob(job->createBackupTask());
            }
        }
    }
}

void JobListWidget::backupItem()
{
    if(sender())
    {
        JobPtr job = qobject_cast<JobListItem*>(sender())->job();
        if(job)
        {
            emit backupJob(job->createBackupTask());
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
    if(sender())
    {
        JobPtr job = qobject_cast<JobListItem*>(sender())->job();
        if(!job->archives().isEmpty())
        {
            ArchivePtr archive = job->archives().first();
            RestoreDialog restoreDialog(archive, this);
            if( QDialog::Accepted == restoreDialog.exec())
                emit restoreArchive(archive, restoreDialog.getOptions());
        }
    }
}

void JobListWidget::deleteItem()
{
    JobListItem* jobItem = qobject_cast<JobListItem*>(sender());
    if(jobItem)
    {
        JobPtr job = jobItem->job();
        QMessageBox::StandardButton confirmJobDelete = QMessageBox::question(this, tr("Confirm action")
                                                                   , tr("Are you sure you want to delete job \"%1\" (this cannot be undone)?").arg(job->name()));
        if(confirmJobDelete == QMessageBox::Yes)
        {
            if(!job->archives().isEmpty())
            {
                QMessageBox::StandardButton confirmJobArchivesDelete = QMessageBox::question(this, tr("Confirm action")
                                                                           , tr("Also delete %1 archives pertaining to this job (this cannot be undone)?").arg(job->archives().count()));
                if(confirmJobArchivesDelete == QMessageBox::Yes)
                    emit deleteJobArchives(job->archives());
            }
            job->purge();
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

void JobListWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(!selectedItems().isEmpty())
            clearSelection();
        else
            QListWidget::keyReleaseEvent(event);
        break;
    default:
        QListWidget::keyReleaseEvent(event);
    }
}

QList<JobPtr> JobListWidget::getStoredJobs()
{
    QList<JobPtr> jobs;
    PersistentStore& store = PersistentStore::instance();
    if(!store.initialized())
    {
        DEBUG << "PersistentStore was not initialized properly.";
        return jobs;
    }
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select name from jobs")))
    {
        DEBUG << query.lastError().text();
        return jobs;
    }
    if(!query.exec())
    {
        DEBUG << query.lastError().text();
        return jobs;
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



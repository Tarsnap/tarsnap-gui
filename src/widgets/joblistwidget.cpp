#include "joblistwidget.h"
#include "joblistitem.h"
#include "restoredialog.h"
#include "debug.h"

#include <QMessageBox>

JobListWidget::JobListWidget(QWidget *parent) : QListWidget(parent)
{
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

    QMessageBox::StandardButton confirm = QMessageBox::question(this, tr("Confirm action")
                                                               , tr("Initiate backup for the %1 selected job(s)?").arg(selectedItems().count()));
    if(confirm == QMessageBox::Yes)
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

void JobListWidget::selectJob(JobPtr job)
{
    if(job)
        selectJobByRef(job->objectKey());
}

void JobListWidget::selectJobByRef(QString jobRef)
{
    if(jobRef.isEmpty())
        return;

    clearSelection();

    JobListItem* jobItem = static_cast<JobListItem*>(this->currentItem());
    if(jobItem && (jobItem->job()->objectKey() == jobRef))
    {
        emit displayJobDetails(jobItem->job());
        return;
    }
    for(int i = 0; i < this->count(); ++i)
    {
        jobItem = static_cast<JobListItem*>(this->item(i));
        if(jobItem && (jobItem->job()->objectKey() == jobRef))
        {
            this->setCurrentItem(jobItem);
            emit displayJobDetails(jobItem->job());
            return;
        }
    }
}

void JobListWidget::backupAllJobs()
{
    for(int i = 0; i < this->count(); ++i)
    {
        JobPtr job = static_cast<JobListItem*>(this->item(i))->job();
        emit backupJob(job->createBackupTask());
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
        bool purgeArchives = false;
        JobPtr job = jobItem->job();
        QMessageBox::StandardButton confirm = QMessageBox::question(this, tr("Confirm action")
                                                                   , tr("Are you sure you want to delete job \"%1\" (this cannot be undone)?").arg(job->name()));
        if(confirm == QMessageBox::Yes)
        {
            if(!job->archives().isEmpty())
            {
                QMessageBox::StandardButton confirmArchives= QMessageBox::question(this, tr("Confirm action")
                                                                           , tr("Also delete %1 archives pertaining to this job (this cannot be undone)?").arg(job->archives().count()));
                if(confirmArchives == QMessageBox::Yes)
                    purgeArchives = true;
            }
            emit deleteJob(job, purgeArchives);
            QListWidgetItem *item = this->takeItem(this->row(jobItem));
            if(item)
                delete item;
        }
    }
}

void JobListWidget::addJobs(QMap<QString, JobPtr> jobs)
{
    clear();
    foreach(JobPtr job, jobs)
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

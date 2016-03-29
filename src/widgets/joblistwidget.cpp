#include "joblistwidget.h"
#include "debug.h"
#include "restoredialog.h"

#include <QMessageBox>

JobListWidget::JobListWidget(QWidget *parent) : QListWidget(parent)
{
    connect(this, &QListWidget::itemActivated, [&](QListWidgetItem *item) {
        emit displayJobDetails(static_cast<JobListItem *>(item)->job());
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

    auto confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Initiate backup for the %1 selected job(s)?")
                                  .arg(selectedItems().count()));
    if(confirm == QMessageBox::Yes)
    {
        foreach(QListWidgetItem *item, selectedItems())
        {
            if(item->isSelected())
            {
                JobPtr job = static_cast<JobListItem *>(item)->job();
                emit   backupJob(job->createBackupTask());
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

    JobListItem *jobItem = static_cast<JobListItem *>(currentItem());
    if(!(jobItem && (jobItem->job()->objectKey() == jobRef)))
    {
        jobItem = nullptr;
        for(int i = 0; i < count(); ++i)
        {
            jobItem = static_cast<JobListItem *>(item(i));
            if(jobItem && (jobItem->job()->objectKey() == jobRef))
                break;
            jobItem = nullptr;
        }
    }
    if(jobItem)
    {
        clearSelection();
        setCurrentItem(jobItem);
        emit displayJobDetails(jobItem->job());
        scrollToItem(currentItem(), QAbstractItemView::EnsureVisible);
    }
}

void JobListWidget::backupAllJobs()
{
    for(int i = 0; i < count(); ++i)
    {
        JobPtr job = static_cast<JobListItem *>(item(i))->job();
        emit   backupJob(job->createBackupTask());
    }
}

void JobListWidget::backupItem()
{
    if(sender())
    {
        JobPtr job = qobject_cast<JobListItem *>(sender())->job();
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
        emit displayJobDetails(qobject_cast<JobListItem *>(sender())->job());
    }
}

void JobListWidget::restoreItem()
{
    if(sender())
    {
        JobPtr job = qobject_cast<JobListItem *>(sender())->job();
        if(!job->archives().isEmpty())
        {
            ArchivePtr    archive = job->archives().first();
            RestoreDialog restoreDialog(archive, this);
            if(QDialog::Accepted == restoreDialog.exec())
                emit restoreArchive(archive, restoreDialog.getOptions());
        }
    }
}

void JobListWidget::deleteItem()
{
    execDeleteJob(qobject_cast<JobListItem *>(sender()));
}

void JobListWidget::execDeleteJob(JobListItem *jobItem)
{
    if(jobItem)
    {
        bool   purgeArchives = false;
        JobPtr job     = jobItem->job();
        auto   confirm =
            QMessageBox::question(this, tr("Confirm action"),
                                  tr("Are you sure you want to delete job "
                                     "\"%1\" (this cannot be undone)?")
                                      .arg(job->name()));
        if(confirm == QMessageBox::Yes)
        {
            if(!job->archives().isEmpty())
            {
                auto confirmArchives =
                    QMessageBox::question(this, tr("Confirm action"),
                                          tr("Also delete %1 archives "
                                             "pertaining to this job (this "
                                             "cannot be undone)?")
                                              .arg(job->archives().count()));
                if(confirmArchives == QMessageBox::Yes)
                    purgeArchives = true;
            }
            emit deleteJob(job, purgeArchives);
            delete jobItem;
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
        connect(item, &JobListItem::requestBackup, this,
                &JobListWidget::backupItem);
        connect(item, &JobListItem::requestInspect, this,
                &JobListWidget::inspectItem);
        connect(item, &JobListItem::requestRestore, this,
                &JobListWidget::restoreItem);
        connect(item, &JobListItem::requestDelete, this,
                &JobListWidget::deleteItem);
        insertItem(count(), item);
        setItemWidget(item, item->widget());
    }
}

void JobListWidget::inspectSelectedItem()
{
    if(!selectedItems().isEmpty())
        emit displayJobDetails(static_cast<JobListItem *>(selectedItems().first())->job());
}

void JobListWidget::restoreSelectedItem()
{
    if(!selectedItems().isEmpty())
    {
        JobPtr job = static_cast<JobListItem *>(selectedItems().first())->job();
        if(!job->archives().isEmpty())
        {
            ArchivePtr    archive = job->archives().first();
            RestoreDialog restoreDialog(archive, this);
            if(QDialog::Accepted == restoreDialog.exec())
                emit restoreArchive(archive, restoreDialog.getOptions());
        }
    }
}

void JobListWidget::deleteSelectedItem()
{
    if(!selectedItems().isEmpty())
        execDeleteJob(static_cast<JobListItem *>(selectedItems().first()));
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

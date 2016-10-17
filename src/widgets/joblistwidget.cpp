#include "joblistwidget.h"
#include "debug.h"
#include "restoredialog.h"

#include <QMessageBox>
#include <QKeyEvent>

JobListWidget::JobListWidget(QWidget *parent) : QListWidget(parent)
{
    connect(this, &QListWidget::itemActivated, [&](QListWidgetItem *item) {
        emit displayJobDetails(static_cast<JobListWidgetItem *>(item)->job());
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
                JobPtr job = static_cast<JobListWidgetItem *>(item)->job();
                emit backupJob(job);
            }
        }
    }
}

void JobListWidget::selectJob(JobPtr job)
{
    if(job)
    {
        for(int i = 0; i < count(); ++i)
        {
            JobListWidgetItem *jobItem = static_cast<JobListWidgetItem *>(item(i));
            if(jobItem && (jobItem->job()->objectKey() == job->objectKey()))
            {
                clearSelection();
                setCurrentItem(jobItem);
                scrollToItem(currentItem(), QAbstractItemView::EnsureVisible);
            }
        }
    }
}

void JobListWidget::inspectJobByRef(QString jobRef)
{
    if(!jobRef.isEmpty())
    {
        for(int i = 0; i < count(); ++i)
        {
            JobListWidgetItem *jobItem = static_cast<JobListWidgetItem *>(item(i));
            if(jobItem && (jobItem->job()->objectKey() == jobRef))
                emit displayJobDetails(jobItem->job());
        }
    }
}

void JobListWidget::backupAllJobs()
{
    for(int i = 0; i < count(); ++i)
    {
        JobPtr job = static_cast<JobListWidgetItem *>(item(i))->job();
        emit backupJob(job);
    }
}

void JobListWidget::backupItem()
{
    if(sender())
    {
        JobPtr job = qobject_cast<JobListWidgetItem *>(sender())->job();
        if(job)
            emit backupJob(job);
    }
}

void JobListWidget::inspectItem()
{
    if(sender())
    {
        emit displayJobDetails(qobject_cast<JobListWidgetItem *>(sender())->job());
    }
}

void JobListWidget::restoreItem()
{
    if(sender())
    {
        JobPtr job = qobject_cast<JobListWidgetItem *>(sender())->job();
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
    execDeleteJob(qobject_cast<JobListWidgetItem *>(sender()));
}

void JobListWidget::execDeleteJob(JobListWidgetItem *jobItem)
{
    if(!jobItem)
        return;

    JobPtr job   = jobItem->job();
    auto confirm = QMessageBox::question(this, tr("Confirm action"),
                                         tr("Are you sure you want to delete job"
                                            " \"%1\" (this cannot be undone)?")
                                         .arg(job->name()));
    if(confirm == QMessageBox::Yes)
    {
        bool purgeArchives = false;
        if(!job->archives().isEmpty())
        {
            auto confirmArchives = QMessageBox::question(this,
                                          tr("Confirm action"),
                                          tr("Also delete %1 archives "
                                             "belonging to this job (this "
                                             "cannot be undone)?")
                                          .arg(job->archives().count()));
            if(confirmArchives == QMessageBox::Yes)
                purgeArchives = true;
        }
        emit deleteJob(job, purgeArchives);
        delete jobItem;
    }
}

void JobListWidget::setJobs(QMap<QString, JobPtr> jobs)
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
        JobListWidgetItem *item = new JobListWidgetItem(job);
        connect(item, &JobListWidgetItem::requestBackup, this,
                &JobListWidget::backupItem);
        connect(item, &JobListWidgetItem::requestInspect, this,
                &JobListWidget::inspectItem);
        connect(item, &JobListWidgetItem::requestRestore, this,
                &JobListWidget::restoreItem);
        connect(item, &JobListWidgetItem::requestDelete, this,
                &JobListWidget::deleteItem);
        insertItem(count(), item);
        setItemWidget(item, item->widget());
    }
}

void JobListWidget::inspectSelectedItem()
{
    if(!selectedItems().isEmpty())
        emit displayJobDetails(
            static_cast<JobListWidgetItem *>(selectedItems().first())->job());
}

void JobListWidget::restoreSelectedItem()
{
    if(!selectedItems().isEmpty())
    {
        JobPtr job = static_cast<JobListWidgetItem *>(selectedItems().first())->job();
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
        execDeleteJob(static_cast<JobListWidgetItem *>(selectedItems().first()));
}

void JobListWidget::setFilter(QString regex)
{
    clearSelection();
    QRegExp rx(regex, Qt::CaseInsensitive, QRegExp::Wildcard);
    for(int i = 0; i < count(); ++i)
    {
        JobListWidgetItem *jobItem =
            static_cast<JobListWidgetItem *>(item(i));
        if(jobItem)
        {
            if(jobItem->job()->name().contains(rx))
                jobItem->setHidden(false);
            else
                jobItem->setHidden(true);
        }
    }
}

void JobListWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Delete:
        deleteSelectedItem();
        break;
    case Qt::Key_Escape:
        if(!selectedItems().isEmpty())
            clearSelection();
        else
            QListWidget::keyPressEvent(event);
        break;
    default:
        QListWidget::keyPressEvent(event);
    }
}

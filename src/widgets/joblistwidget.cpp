#include "joblistwidget.h"

WARNINGS_DISABLE
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRegExp>
#include <Qt>
WARNINGS_ENABLE

#include "messages/archiverestoreoptions.h"

#include "debug.h"
#include "joblistwidgetitem.h"
#include "persistentmodel/job.h"
#include "widgets/restoredialog.h"

JobListWidget::JobListWidget(QWidget *parent)
    : QListWidget(parent), _filter(new QRegExp)
{
    // Set up filtering job names.
    _filter->setCaseSensitivity(Qt::CaseInsensitive);
    _filter->setPatternSyntax(QRegExp::Wildcard);

    // Connection for showing info about a Job.
    connect(this, &QListWidget::itemActivated, [this](QListWidgetItem *item) {
        emit displayJobDetails(static_cast<JobListWidgetItem *>(item)->job());
    });
}

JobListWidget::~JobListWidget()
{
    clear();
    delete _filter;
}

void JobListWidget::backupSelectedItems()
{
    // Bail (if applicable).
    if(selectedItems().isEmpty())
        return;

    // Confirm that the user wants to create new archive(s).
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Initiate backup for the %1 selected job(s)?")
                                  .arg(selectedItems().count()));
    if(confirm != QMessageBox::Yes)
        return;

    // Create a new archive for each selected Job.
    for(QListWidgetItem *item : selectedItems())
    {
        JobPtr job = static_cast<JobListWidgetItem *>(item)->job();
        emit   backupJob(job);
    }
}

void JobListWidget::selectJob(JobPtr job)
{
    // Bail (if applicable).
    if(!job)
    {
        DEBUG << "Null JobPtr passed.";
        return;
    }

    // Find the item representing the Job, and select it.
    for(int i = 0; i < count(); ++i)
    {
        JobListWidgetItem *jobItem = static_cast<JobListWidgetItem *>(item(i));
        if(jobItem && (jobItem->job()->objectKey() == job->objectKey()))
        {
            clearSelection();
            setCurrentItem(jobItem);
            scrollToItem(currentItem(), QAbstractItemView::EnsureVisible);
            break;
        }
    }
}

void JobListWidget::inspectJobByRef(const QString &jobRef)
{
    // Bail (if applicable).
    if(jobRef.isEmpty())
        return;

    // Find the item representing the Job, and display its details.
    for(int i = 0; i < count(); ++i)
    {
        JobListWidgetItem *jobItem = static_cast<JobListWidgetItem *>(item(i));
        if(jobItem && (jobItem->job()->objectKey() == jobRef))
            emit displayJobDetails(jobItem->job());
    }
}

void JobListWidget::backupAllJobs()
{
    // Start a new archive for all jobs.
    for(int i = 0; i < count(); ++i)
    {
        JobPtr job = static_cast<JobListWidgetItem *>(item(i))->job();
        emit   backupJob(job);
    }
}

void JobListWidget::backupItem()
{
    // Bail (if applicable).
    if(!sender())
        return;

    // Start a new archive for this job.
    JobPtr job = qobject_cast<JobListWidgetItem *>(sender())->job();
    if(job)
        emit backupJob(job);
}

void JobListWidget::inspectItem()
{
    // Bail (if applicable).
    if(!sender())
        return;

    // Display details about the job.
    emit displayJobDetails(qobject_cast<JobListWidgetItem *>(sender())->job());
}

void JobListWidget::restoreItem()
{
    // Bail (if applicable).
    if(!sender())
        return;

    // Get the Job.
    JobPtr job = qobject_cast<JobListWidgetItem *>(sender())->job();

    // Bail (if applicable).
    if(job->archives().isEmpty())
        return;

    // Get the latest archive belonging to the Job.
    ArchivePtr archive = job->archives().first();

    // Launch the RestoreDialog.
    RestoreDialog *restoreDialog = new RestoreDialog(this, archive);
    connect(restoreDialog, &RestoreDialog::accepted, [this, restoreDialog] {
        emit restoreArchive(restoreDialog->archive(),
                            restoreDialog->getOptions());
    });
    restoreDialog->show();
}

void JobListWidget::deleteItem()
{
    execDeleteJob(qobject_cast<JobListWidgetItem *>(sender()));
}

void JobListWidget::execDeleteJob(JobListWidgetItem *jobItem)
{
    // Bail (if applicable).
    if(!jobItem)
    {
        DEBUG << "Null JobListWidgetItem passed.";
        return;
    }

    // Get the Job.
    JobPtr job = jobItem->job();

    // Confirm that the user wants to delete the Job.
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Are you sure you want to delete job \"%1\" "
                                 "(this cannot be undone)?")
                                  .arg(job->name()));
    if(confirm != QMessageBox::Yes)
        return;

    // Confirm if the user wants to remove archives as well.
    bool purgeArchives = false;
    if(!job->archives().isEmpty())
    {
        QMessageBox::StandardButton delArchives =
            QMessageBox::question(this, tr("Confirm action"),
                                  tr("Also delete %1 archives "
                                     "belonging to this job "
                                     "(this cannot be undone)?")
                                      .arg(job->archives().count()));
        if(delArchives == QMessageBox::Yes)
            purgeArchives = true;
    }

    // Begin deleting the job (and possibly archives as well).
    emit deleteJob(job, purgeArchives);
    delete jobItem;

    // Notify about the number of visible items.
    emit countChanged(count(), visibleItemsCount());
}

int JobListWidget::visibleItemsCount()
{
    int count = 0;
    for(QListWidgetItem *item : findItems("*", Qt::MatchWildcard))
    {
        if(item && !item->isHidden())
            count++;
    }
    return count;
}

void JobListWidget::setJobs(const QMap<QString, JobPtr> &jobs)
{
    setUpdatesEnabled(false);
    clear();
    for(const JobPtr &job : jobs)
    {
        addJob(job);
    }
    setUpdatesEnabled(true);
}

void JobListWidget::addJob(JobPtr job)
{
    // Bail (if applicable).
    if(!job)
    {
        DEBUG << "Null JobPtr passed.";
        return;
    }

    // Create new item.
    JobListWidgetItem *item = new JobListWidgetItem(job);
    connect(item, &JobListWidgetItem::requestBackup, this,
            &JobListWidget::backupItem);
    connect(item, &JobListWidgetItem::requestInspect, this,
            &JobListWidget::inspectItem);
    connect(item, &JobListWidgetItem::requestRestore, this,
            &JobListWidget::restoreItem);
    connect(item, &JobListWidgetItem::requestDelete, this,
            &JobListWidget::deleteItem);

    // Add it to the end of the list.
    insertItem(count(), item);
    setItemWidget(item, item->widget());

    // Check it against the name filter.
    item->setHidden(!job->name().contains(*_filter));

    // Notify about the number of visible items.
    emit countChanged(count(), visibleItemsCount());
}

void JobListWidget::inspectSelectedItem()
{
    // Bail (if applicable).
    if(selectedItems().isEmpty())
        return;

    // Display details about the first of the selected items.
    emit displayJobDetails(
        static_cast<JobListWidgetItem *>(selectedItems().first())->job());
}

void JobListWidget::restoreSelectedItem()
{
    // Bail (if applicable).
    if(selectedItems().isEmpty())
        return;

    // Get the first Job amongst the selected items.
    JobPtr job =
        static_cast<JobListWidgetItem *>(selectedItems().first())->job();

    // Bail (if applicable).
    if(job->archives().isEmpty())
        return;

    // Get the latest archive belonging to the Job.
    ArchivePtr archive = job->archives().first();

    // Launch the RestoreDialog.
    RestoreDialog *restoreDialog = new RestoreDialog(this, archive);
    connect(restoreDialog, &RestoreDialog::accepted, [this, restoreDialog] {
        emit restoreArchive(restoreDialog->archive(),
                            restoreDialog->getOptions());
    });
    restoreDialog->show();
}

void JobListWidget::deleteSelectedItem()
{
    // Bail (if applicable).
    if(selectedItems().isEmpty())
        return;

    // Delete the first of the selected items.
    execDeleteJob(static_cast<JobListWidgetItem *>(selectedItems().first()));
}

void JobListWidget::setFilter(const QString &regex)
{
    setUpdatesEnabled(false);

    // Set up filter.
    clearSelection();
    _filter->setPattern(regex);

    // Check jobs against filter.
    for(int i = 0; i < count(); ++i)
    {
        JobListWidgetItem *jobItem = static_cast<JobListWidgetItem *>(item(i));
        if(jobItem)
        {
            if(jobItem->job()->name().contains(*_filter))
                jobItem->setHidden(false);
            else
                jobItem->setHidden(true);
        }
    }
    setUpdatesEnabled(true);

    // Notify about the number of visible items.
    emit countChanged(count(), visibleItemsCount());
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

void JobListWidget::updateIEC()
{
    for(int i = 0; i < count(); ++i)
    {
        JobListWidgetItem *jobItem = static_cast<JobListWidgetItem *>(item(i));
        jobItem->updateIEC();
    }
}

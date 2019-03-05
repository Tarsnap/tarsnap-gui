#ifndef JOBLISTWIDGET_H
#define JOBLISTWIDGET_H

#include "joblistwidgetitem.h"
#include "persistentmodel/job.h"

#include <QListWidget>

/*!
 * \ingroup widgets-specialized
 * \brief The JobListWidget is a QListWidget which displays
 * information about all jobs.
 */
class JobListWidget : public QListWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit JobListWidget(QWidget *parent = nullptr);
    ~JobListWidget();

public slots:
    //! Clears the job list, then sets it to the specified jobs.
    void setJobs(QMap<QString, JobPtr> jobs);
    //! Create new archives for the selected jobs.
    void backupSelectedItems();
    //! Sets the current selection in the list view.
    void selectJob(JobPtr job);
    //! Display detailed information about a specific job.
    //! \param jobRef the name of the job to inspect.
    void inspectJobByRef(QString jobRef);
    //! Create new archives for all jobs.
    void backupAllJobs();
    //! Add a new job to the list.
    void addJob(JobPtr job);
    //! Display detailed information about the first of the selected items.
    void inspectSelectedItem();
    //! Restore the first of the selected jobs.
    void restoreSelectedItem();
    //! Delete the selected job.
    void deleteSelectedItem();
    //! Filter the list of jobs.
    void setFilter(QString regex);

signals:
    //! Notify that the job details should be displayed.
    void displayJobDetails(JobPtr job);
    //! Notify that a new archive should be created for a specific job.
    void backupJob(JobPtr job);
    //! Notify that the specified archive should be restored,
    //! using the user-selected options from the \ref RestoreDialog.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! Notify that the specified job (and possibly archives)
    //! should be deleted.
    //! \param job the job to delete.
    //! \param purgeArchives if true, delete all archives belonging
    //!        to this job.
    void deleteJob(JobPtr job, bool purgeArchives);
    //! Notify the total and visible (not hidden) items count on list change
    //! (item added, removed or hidden).
    void countChanged(int countTotal, int countVisible);

protected:
    //! Handles the delete and escape keys; passes other events on.
    void keyPressEvent(QKeyEvent *event);

private slots:
    void backupItem();
    void inspectItem();
    void restoreItem();
    void deleteItem();

private:
    int  visibleItemsCount();
    void execDeleteJob(JobListWidgetItem *jobItem);

    QRegExp _filter;
};

#endif // JOBLISTWIDGET_H

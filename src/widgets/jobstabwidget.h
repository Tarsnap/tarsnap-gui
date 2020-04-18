#ifndef JOBSTABWIDGET_H
#define JOBSTABWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QMap>
#include <QObject>
#include <QUrl>
#include <QWidget>
WARNINGS_ENABLE

#include "archiverestoreoptions.h"
#include "backuptaskdataptr.h"
#include "messages/archiveptr.h"
#include "messages/jobptr.h"

/* Forward declaration(s). */
namespace Ui
{
class JobsTabWidget;
}
class QEvent;
class QPoint;

/*!
 * \ingroup widgets-main
 * \brief The JobsTabWidget is a QWidget which shows the list of jobs.
 */
class JobsTabWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestJobsTabWidget;
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit JobsTabWidget(QWidget *parent = nullptr);
    ~JobsTabWidget();

public slots:
    //! The user clicked on the "add job / save job" button, or selected the
    //! menu item.
    void addJobClicked();
    //! Create a new job from the Backup list.
    void createNewJob(QList<QUrl> urls, QString name);
    //! Show detailed information about a job.
    void displayJobDetails(JobPtr job);

signals:
    //! Passes info.
    void jobAdded(JobPtr job);
    //! Archives which match the previously-given search string.
    void matchingArchives(QList<ArchivePtr> archives);
    //! Notifies about a deleted job from the JobWidget or JobListWidget.
    void deleteJob(JobPtr job, bool purgeArchives);
    //! Search for all matching Archive objects which were created by a Job.
    //! \param jobPrefix prefix of the Archive names to match.
    void findMatchingArchives(QString jobPrefix);
    //! Begin tarsnap -x -f \<name\>, with options.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! Show detailed information about an archive.
    void displayInspectArchive(ArchivePtr archive);
    //! Passes info from the ArchiveListWidget or JobWidget to the TaskManager.
    void deleteArchives(QList<ArchivePtr> archives);
    //! Create new archives for the selected job(s).
    void backupSelectedItems();
    //! Delete the selected job.
    void deleteSelectedItem();
    //! Restore the first of the selected jobs.
    void restoreSelectedItem();
    //! Display detailed information about the first of the selected items.
    void inspectSelectedItem();
    //! Passes the list of all Job objects to the JobListWidget.
    void jobList(QMap<QString, JobPtr>);
    //! Display detailed information about a specific job.
    void jobInspectByRef(QString jobRef);
    //! Begin tarsnap -c -f \<name\>
    void backupNow(BackupTaskDataPtr backupTaskData);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);
    //! Handles the escape key; passes other events on.
    void keyPressEvent(QKeyEvent *event);

private slots:
    void addDefaultJobs();
    void hideJobDetails();
    void showJobsListMenu(const QPoint &pos);
    void backupJob(JobPtr job);

private:
    Ui::JobsTabWidget *_ui;

    void updateUi();
    void loadSettings();
};

#endif // JOBSTABWIDGET_H

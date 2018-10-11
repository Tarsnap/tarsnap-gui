#ifndef JOBSTABWIDGET_H
#define JOBSTABWIDGET_H

#include "ui_jobstabwidget.h"

#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWidget>

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

    // TODO: this is a hack for refactoring
    QToolButton *temp_addJobButton() { return _ui.addJobButton; }

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
    //! \param jobPrefix: prefix of the Archive names to match.
    void findMatchingArchives(QString jobPrefix);
    //! Begin tarsnap -x -f \<name\>, with options.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! Show detailed information about an archive.
    void displayInspectArchive(ArchivePtr archive);
    //! Passes info from the ArchiveListWidget or JobWidget to the TaskManager.
    void deleteArchives(QList<ArchivePtr> archives);
    //! Create a new archive from an existing Job.
    void backupJob(JobPtr job);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);
    //! Handles the escape key; passes other events on.
    void keyPressEvent(QKeyEvent *event);

private slots:
    void addDefaultJobs();
    void hideJobDetails();

private:
    Ui::JobsTabWidget _ui;

    void updateUi();
    void loadSettings();
};

#endif // JOBSTABWIDGET_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QUuid>
#include <QVector>
#include <QWidget>
WARNINGS_ENABLE

#include "messages/archiveptr.h"
#include "messages/archiverestoreoptions.h"
#include "messages/backuptaskdataptr.h"
#include "messages/jobptr.h"
#include "messages/notification_info.h"
#include "messages/tarsnaperror.h"
#include "messages/taskstatus.h"

/* Forward declaration(s). */
namespace Ui
{
class MainWindow;
}
class ArchivesTabWidget;
class BackupTabWidget;
class BaseTask;
class JobsTabWidget;
class HelpWidget;
class QEvent;
class QMenuBar;
class SettingsWidget;
class StopTasksDialog;
struct LogEntry;

/*!
 * \ingroup widgets-main
 * \brief The MainWindow is a QMainWindow which is the main application window.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    //! Initialization routines: Check days elapsed since last credit update,
    //! application paths, and update archive list.
    void initializeMainWindow();
    //! Set the statusbar message.
    //! \param message display this text
    //! \param detail display this text as a mouse-over tooltip.
    void updateStatusMessage(const QString &message,
                             const QString &detail = "");
    //! Update the global Tarsnap --print-stats values in the Settings tab.
    void overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
                             quint64 sizeUniqueTotal,
                             quint64 sizeUniqueCompressed,
                             quint64 archiveCount);
    //! Update the Tarsnap version number, and store it in the settings.
    //! \anchor tarsnapVersionResponse
    void tarsnapVersionResponse(TaskStatus     status,
                                const QString &versionString);
    //! Display (and raise) the MainWindow (if minimized or hidden).
    void notificationRaise();
    //! Display an explanation of a tarsnap CLI error.
    void tarsnapError(TarsnapError error);
    //! Append a new entry to the journal.
    void appendToJournalLog(const LogEntry &log);
    //! Reset the current Journal using log.
    void setJournal(const QVector<LogEntry> &log);
    //! Save the Tarsnap key ID.
    void saveKeyId(const QString &key_filename, quint64 id);

    //! Update the number of tasks;
    void updateNumTasks(bool backupRunning, int numRunning, int numQueued);

    //! Handle a clicked notification message.
    void handle_notification_clicked(enum message_type type,
                                     const QString    &data);

signals:
    //! Begin tarsnap -c -f \<name\>
    void backupNow(BackupTaskDataPtr backupTaskData);
    //! Begin tarsnap --list-archives
    void getArchives();
    //! Passes the list of all Archive objects to the ArchiveListWidget.
    void archiveList(QList<ArchivePtr> archives);
    //! Passes the creation of a new Archive to the ArchiveListWidget.
    void addArchive(ArchivePtr archive);
    //! Passes info from the ArchiveListWidget or JobDetailsWidget to the
    //! TaskManager.
    void deleteArchives(QList<ArchivePtr> archives);
    //! Begin tarsnap --print-stats -f \<name\>
    void loadArchiveStats(ArchivePtr archive);
    //! Begin tarsnap --tv -f \<name\>
    void loadArchiveContents(ArchivePtr archive);
    //! Begin tarsnap --print-stats
    void getOverallStats();
    //! Begin tarsnap --fsck or --fsck-prune
    void repairCache(bool prune);
    //! The confirmation has been accepted and the timer has finished; go
    //! ahead and nuke all archives.
    void nukeArchives();
    //! Begin tarsnap -x -f \<name\>, with options.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! The user has confirmed that the settings can be wiped; go ahead
    //! and run the setup wizard again.
    void runSetupWizard();
    //! Stop / interrupt / terminate / dequeue tasks.
    //! \param interrupt Kill the first task.  \warning MacOS X only.  (?)
    //! \param running Stop all running tasks.
    //! \param queued Remove all tasks from the queue.
    void stopTasks(bool interrupt, bool running, bool queued);
    //! Begin tarsnap --version
    //! \anchor tarsnapVersionRequested
    void tarsnapVersionRequested(const QString &tarsnapPath);
    //! Clear all Journal entries.
    void clearJournal();
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(const QString &key_filename);

    // Backup tab
    //! Create a new job with the given urls and name.
    void morphBackupIntoJob(QList<QUrl> urls, const QString &name);

    // Job tab
    //! Passes the list of all Job objects to the JobListWidget.
    void jobList(QMap<QString, JobPtr>);
    //! Notifies about a deleted job from the JobDetailsWidget or JobListWidget.
    void deleteJob(JobPtr job, bool purgeArchives);
    //! Passes info from the JobDetailsWidget to the TaskManager.
    void jobAdded(JobPtr job);
    //! Search for all matching Archive objects which were created by a Job.
    //! \param jobPrefix prefix of the Archive names to match.
    void findMatchingArchives(const QString &jobPrefix);
    //! Archives which match the previously-given search string.
    void matchingArchives(QList<ArchivePtr> archives);

    //! Is the Backup tab ready to create an archive?
    void validBackupTab(bool valid);

    //! We have a task to perform in the background.
    void taskRequested(BaseTask *task);
    //! We would like to cancel a task.
    void cancelTaskRequested(BaseTask *task, const QUuid uuid);

protected:
    //! Draw the Tarsnap logo in the top-right corner.
    void paintEvent(QPaintEvent *) override;
    //! Handles the escape key; passes other events on.
    void keyPressEvent(QKeyEvent *event) override;
    //! Start checking whether there are running tasks.
    void closeEvent(QCloseEvent *event) override;
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private slots:
    void setupMenuBar();
    void displayInspectArchive(ArchivePtr archive);
    void commitSettings();
    void mainTabChanged(int index);

    // Backup tab
    void backupTabValidStatus(bool valid);
    // Open a file dialog to add (multiple) items.
    void browseForBackupItems();

    // Jobs tab
    void displayJobDetails(JobPtr job);
    void createNewJob(const QList<QUrl> &urls, const QString &name);

    void nonquitStopTasks();

private:
    Ui::MainWindow *_ui;
    int             _minWidth;
    QMenuBar       *_menuBar;
    bool            _aboutToQuit;
    bool            _backupTaskRunning;
    int             _runningTasks;
    int             _queuedTasks;

    StopTasksDialog   *_stopTasksDialog;
    BackupTabWidget   *_backupTabWidget;
    ArchivesTabWidget *_archivesTabWidget;
    JobsTabWidget     *_jobsTabWidget;
    SettingsWidget    *_settingsWidget;
    HelpWidget        *_helpWidget;

    //! Switch to displaying a specific tab.
    void displayTab(QWidget *widget);

    void updateUi();
    // Load saved application settings.
    void loadSettings();

    void connectSettingsWidget();

    // Display detailed information about a specific job.
    void jobInspectByRef(const QString &jobRef);
};

#endif // MAINWINDOW_H

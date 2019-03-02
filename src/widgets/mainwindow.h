#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "archivestabwidget.h"
#include "backuptabwidget.h"
#include "helpwidget.h"
#include "jobstabwidget.h"
#include "settingswidget.h"

#include "persistentmodel/journal.h"
#include "scheduling.h"
#include "tarsnapaccount.h"
#include "taskmanager.h"

#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

namespace Ui
{
class MainWindow;
}

/*!
 * \ingroup widgets-main
 * \brief The MainWindow is a QWidget which is the main application window.
 */
class MainWindow : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    //! Initialization routines: Check days elapsed since last credit update,
    //! application paths, and update archive list.
    void initializeMainWindow();
    //! Set the statusbar message.
    //! \param message display this text
    //! \param detail display this text as a mouse-over tooltip.
    void updateStatusMessage(QString message, QString detail = "");
    //! Indicate if the application is busy running Tarsnap chores or not.
    void updateLoadingAnimation(bool idle);
    //! Update the global Tarsnap --print-stats values in the Settings tab.
    void overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
                             quint64 sizeUniqueTotal,
                             quint64 sizeUniqueCompressed, quint64 archiveCount);
    //! Update the Tarsnap version number, and store it in the settings.
    void updateTarsnapVersion(QString versionString);
    //! Display (and raise) the MainWindow (if minimized or hidden).
    void notificationRaise();
    //! We received information about tasks after receiving a closeEvent.
    void closeWithTaskInfo(bool backupTaskRunning, int runningTasks,
                           int queuedTasks);
    //! Display an explanation of a tarsnap CLI error.
    void tarsnapError(TarsnapError error);
    //! Append a new entry to the journal.
    void appendToJournalLog(LogEntry log);
    //! Append a new entry to the console log.
    void appendToConsoleLog(const QString &log);
    //! Reset the current Journal using log.
    void setJournal(QVector<LogEntry> log);
    //! Save the Tarsnap key ID.
    void saveKeyId(QString key_filename, quint64 id);

    //! Update the simulation icon.
    void updateSimulationIcon(int state);
    //! Update the number of tasks;
    void updateNumTasks(int numRunning, int numQueued);

signals:
    //! Begin tarsnap -c -f \<name\>
    void backupNow(BackupTaskPtr backupTask);
    //! Begin tarsnap --list-archives
    void getArchives();
    //! Passes the list of all Archive objects to the ArchiveListWidget.
    void archiveList(QList<ArchivePtr> archives);
    //! Passes the creation of a new Archive to the ArchiveListWidget.
    void addArchive(ArchivePtr archive);
    //! Passes info from the ArchiveListWidget or JobWidget to the TaskManager.
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
    void getTarsnapVersion(QString tarsnapPath);
    //! Query whether there are any running tasks; will trigger a taskInfo
    //! signal which is received by \ref closeWithTaskInfo.
    void getTaskInfo();
    //! Clear all Journal entries.
    void clearJournal();
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(QString key_filename);

    // Backup tab
    //! Create a new job with the given urls and name.
    void morphBackupIntoJob(QList<QUrl> urls, QString name);

    // Job tab
    //! Passes the list of all Job objects to the JobListWidget.
    void jobList(QMap<QString, JobPtr>);
    //! Notifies about a deleted job from the JobWidget or JobListWidget.
    void deleteJob(JobPtr job, bool purgeArchives);
    //! Passes info from the JobWidget to the TaskManager.
    void jobAdded(JobPtr job);
    //! Search for all matching Archive objects which were created by a Job.
    //! \param jobPrefix prefix of the Archive names to match.
    void findMatchingArchives(QString jobPrefix);
    //! Archives which match the previously-given search string.
    void matchingArchives(QList<ArchivePtr> archives);

    //! Is the Backup tab ready to create an archive?
    void validBackupTab(bool valid);

protected:
    //! Draw the Tarsnap logo in the top-right corner.
    void paintEvent(QPaintEvent *);
    //! Handles the escape key; passes other events on.
    void keyPressEvent(QKeyEvent *event);
    //! Start checking whether there are running tasks.
    void closeEvent(QCloseEvent *event);
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

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
    void createNewJob(QList<QUrl> urls, QString name);

private:
    Ui::MainWindow *_ui;
    int             _minWidth;
    QMenuBar *      _menuBar;
    bool            _aboutToQuit;
    QMessageBox     _stopTasksDialog;
    int             _runningTasks;
    int             _queuedTasks;

    QPlainTextEdit *_consoleLog;

    BackupTabWidget   _backupTabWidget;
    ArchivesTabWidget _archivesTabWidget;
    JobsTabWidget     _jobsTabWidget;
    SettingsWidget    _settingsWidget;
    HelpWidget        _helpWidget;

    //! Switch to displaying a specific tab.
    void displayTab(QWidget *widget);

    void updateUi();
    // Load saved application settings.
    void loadSettings();
    // Prompt user to clarify whether to stop background tasks; if so, quits
    // the app.  Also used when quitting the application while active or
    // background tasks are queued.
    void displayStopTasksDialog(bool backupTaskRunning, int runningTasks,
                                int queuedTasks);

    void connectSettingsWidget();

    // Display detailed information about a specific job.
    void jobInspectByRef(QString jobRef);
};

#endif // MAINWINDOW_H

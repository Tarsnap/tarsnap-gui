#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "persistentmodel/journal.h"
#include "scheduling.h"
#include "tarsnapaccount.h"
#include "taskmanager.h"
#include "ui_mainwindow.h"

#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

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
    //! \param message: display this text
    //! \param detail: display this text as a mouse-over tooltip.
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
    //! Reset the current Journal using _log.
    void setJournal(QVector<LogEntry> _log);
    //! Save the Tarsnap key ID.
    void saveKeyId(QString key_filename, quint64 id);
    //! Create a new archive from an existing Job.
    void backupJob(JobPtr job);

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
    void stopTasks(bool interrupt, bool running, bool queued);
    //! Passes the list of all Job objects to the JobListWidget.
    void jobList(QMap<QString, JobPtr>);
    //! Notifies about a deleted job from the JobWidget or JobListWidget.
    void deleteJob(JobPtr job, bool purgeArchives);
    //! Begin tarsnap --version
    void getTarsnapVersion(QString tarsnapPath);
    //! Query whether there are any running tasks; will trigger a taskInfo
    //! signal which is received by \ref closeWithTaskInfo.
    void getTaskInfo();
    //! Passes info from the JobWidget to the TaskManager.
    void jobAdded(JobPtr job);
    //! Clear all Journal entries.
    void clearJournal();
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(QString key_filename);
    //! Search for all matching Archive objects which were created by a Job.
    //! \param jobPrefix: prefix of the Archive names to match.
    void findMatchingArchives(QString jobPrefix);
    //! Archives which match the previously-given search string.
    void matchingArchives(QList<ArchivePtr> archives);

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
    void updateBackupItemTotals(quint64 count, quint64 size);
    void displayInspectArchive(ArchivePtr archive);
    void commitSettings();
    bool validateMachineKeyPath();
    bool validateTarsnapPath();
    bool validateTarsnapCache();
    bool validateAppDataDir();
    void nukeTimerFired();
    void browseForBackupItems();
    void appendTimestampCheckBoxToggled(bool checked);
    void backupButtonClicked();
    void accountMachineUseHostnameButtonClicked();
    void accountMachineKeyBrowseButtonClicked();
    void tarsnapPathBrowseButtonClicked();
    void tarsnapCacheBrowseButton();
    void appDataButtonClicked();
    void nukeArchivesButtonClicked();
    void runSetupWizardClicked();
    void downloadsDirBrowseButtonClicked();
    void displayJobDetails(JobPtr job);
    void hideJobDetails();
    void addJobClicked();
    void updateAccountCredit(qreal credit, QDate date);
    void updateLastMachineActivity(QStringList activityFields);
    void clearJournalClicked();
    void showArchiveListMenu(const QPoint &pos);
    void showJobsListMenu(const QPoint &pos);
    void addDefaultJobs();
    void createJobClicked();
    void aboutMenuClicked();
    void mainTabChanged(int index);
    void validateBackupTab();
    void enableJobSchedulingButtonClicked();
    void disableJobSchedulingButtonClicked();

private:
    Ui::MainWindow _ui;
    int            _minWidth;
    QMenuBar *     _menuBar;
    QString        _lastTimestamp;
    QTimer         _nukeTimer;
    int            _nukeTimerCount;
    QMessageBox    _nukeCountdown;
    TarsnapAccount _tarsnapAccount;
    bool           _aboutToQuit;
    QString        _helpTabHTML;
    QDialog        _aboutWindow;
    QDialog        _consoleWindow;
    QMessageBox    _stopTasksDialog;

    QPlainTextEdit *_consoleLog;

    void updateUi();
    // Load saved application settings.
    void loadSettings();
    // Prompt user to clarify whether to stop background tasks; if so, quits
    // the app.  Also used when quitting the application while active or
    // background tasks are queued.
    void displayStopTasksDialog(bool backupTaskRunning, int runningTasks,
                                int queuedTasks);
};

#endif // MAINWINDOW_H

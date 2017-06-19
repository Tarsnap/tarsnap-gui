#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "persistentmodel/journal.h"
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

public:
    //! Constructor.
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    //! Load settings from the config file.
    void loadSettings();
    //! Check days elapsed since last credit update, check application paths,
    //! and update archive list.
    void initialize();
    //! Set the statusbar message.
    //! \param message: display this text
    //! \param detail: display this text as a mouse-over tooltip.
    void updateStatusMessage(QString message, QString detail = "");
    //! Show a "waiting for results" indicator (or not).
    void updateLoadingAnimation(bool idle);
    //! Show the Tarsnap --print-stats values in the Settings tab.
    void overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
                             quint64 sizeUniqueTotal,
                             quint64 sizeUniqueCompressed, quint64 archiveCount);
    //! Show the Tarsnap version number, and store it in the settings.
    void updateTarsnapVersion(QString versionString);
    void notificationRaise();
    //! Prompt user to clarify whether to stop background tasks and then
    //! quit the app, or continue the tasks and cancel the "quit" command.
    void displayStopTasks(bool backupTaskRunning, int runningTasks,
                          int queuedTasks);
    //! Display an explanation of a tarsnap CLI error.
    void tarsnapError(TarsnapError error);
    //! Append a new entry to the journal (bottom of the window).
    void appendToJournalLog(LogEntry log);
    //! Append a new entry to the console log (in the Help tab).
    void appendToConsoleLog(const QString &log);
    //! Clears the current Journal, and loads new entries.  Used for
    //! loading the Journal file a file (on app startup).
    void setJournal(QVector<LogEntry> _log);
    void saveKeyId(QString key, int id);
    //! Create a new archive from an existing Job.
    void backupJob(JobPtr job);

signals:
    //! Begin tarsnap -c -f \<name\>
    void backupNow(BackupTaskPtr backupTask);
    //! Begin tarsnap --list-archives
    void getArchives();
    void archiveList(QList<ArchivePtr> archives);
    void addArchive(ArchivePtr archive);
    void deleteArchives(QList<ArchivePtr> archives);
    //! Begin tarsnap --print-stats -f \<name\>
    void loadArchiveStats(ArchivePtr archive);
    //! Begin tarsnap --tv -f \<name\>
    void loadArchiveContents(ArchivePtr archive);
    //! Begin tarsnap --print-stats
    void getOverallStats();
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
    void jobsList(QMap<QString, JobPtr>);
    void deleteJob(JobPtr job, bool purgeArchives);
    //! Begin tarsnap --version
    void getTarsnapVersion(QString tarsnapPath);
    void displayNotification(QString message);
    //! Query whether there are any running tasks; will receive a taskInfo
    //! signal which is received by \ref displayStopTasks.
    void getTaskInfo();
    void jobAdded(JobPtr job);
    //! Clear all Journal entries.
    void clearJournal();
    void getKeyId(QString key);
    void findMatchingArchives(QString jobPrefix);
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
    void showAbout();
    void mainTabChanged(int index);
    void validateBackupTab();
    void enableJobScheduling();
    void disableJobScheduling();

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
    QString	       _helpTabHTML;

    void updateUi();
};

#endif // MAINWINDOW_H

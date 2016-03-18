#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "persistentmodel/journal.h"
#include "tarsnapaccount.h"
#include "taskmanager.h"

#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void backupNow(BackupTaskPtr backupTask);
    void getArchives();
    void archiveList(QList<ArchivePtr> archives);
    void deleteArchives(QList<ArchivePtr> archives);
    void loadArchiveStats(ArchivePtr archive);
    void loadArchiveContents(ArchivePtr archive);
    void getOverallStats();
    void repairCache(bool prune);
    void settingsChanged();
    void purgeArchives();
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void runSetupWizard();
    void stopTasks(bool interrupt, bool running, bool queued);
    void jobsList(QMap<QString, JobPtr>);
    void deleteJob(JobPtr job, bool purgeArchives);
    void getTarsnapVersion(QString tarsnapPath);
    void displayNotification(QString message);
    void getTaskInfo();
    void jobAdded(JobPtr job);
    void clearJournal();
    void logMessage(QString message);
    void getKeyId(QString key);

protected:
    void paintEvent(QPaintEvent *);
    void keyReleaseEvent(QKeyEvent *event);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
//    void mouseDoubleClickEvent(QMouseEvent *event);

public slots:
    void loadSettings();
    void updateStatusMessage(QString message, QString detail = "");
    void updateLoadingAnimation(bool idle);
    void updateSettingsSummary(quint64 sizeTotal, quint64 sizeCompressed,
                               quint64 sizeUniqueTotal,
                               quint64 sizeUniqueCompressed,
                               quint64 archiveCount);
    void updateTarsnapVersion(QString versionString);
    void notificationRaise();
    void displayStopTasks(bool backupTaskRunning, int runningTasks, int queuedTasks);
    void tarsnapError(TarsnapError error);
    void appendToJournalLog(LogEntry log);
    void appendToConsoleLog(const QString& log);
    void setJournal(QVector<LogEntry> _log);
    void saveKeyId(QString key, int id);

private slots:
    void updateBackupItemTotals(quint64 count, quint64 size);
    void displayInspectArchive(ArchivePtr archive);
    void commitSettings();
    void validateMachineKeyPath();
    bool validateTarsnapPath();
    void validateTarsnapCache();
    void purgeTimerFired();
    void browseForBackupItems();
    void appendTimestampCheckBoxToggled(bool checked);
    void backupButtonClicked();
    void accountMachineUseHostnameButtonClicked();
    void accountMachineKeyBrowseButtonClicked();
    void tarsnapPathBrowseButtonClicked();
    void tarsnapCacheBrowseButton();
    void appDataButtonClicked();
    void purgeArchivesButtonClicked();
    void runSetupWizardClicked();
    void expandJournalButtonToggled(bool checked);
    void downloadsDirBrowseButtonClicked();
    void displayJobDetails(JobPtr job);
    void hideJobDetails();
    void addJobClicked();
    void updateAccountCredit(qreal credit, QDate date);
    void updateLastMachineActivity(QStringList activityFields);
    void clearJournalClicked();
    void showArchiveListMenu(const QPoint& pos);
    void showJobsListMenu(const QPoint& pos);
    void addDefaultJobs();
    void setTarsnapVersion(QString versionString);

private:
    Ui::MainWindow *_ui;
    QPixmap         _logo;
    QIcon           _icon;
    QDialog         _aboutWindow;
    QPoint          _windowDragPos;
    QString         _lastTimestamp;
    bool            _useIECPrefixes;
    QTimer          _purgeTimer;
    int             _purgeTimerCount;
    QMessageBox     _purgeCountdownWindow;
    TarsnapAccount  _tarsnapAccount;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "taskmanager.h"
#include "tarsnapaccount.h"

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QMenuBar>
#include <QMessageBox>

namespace Ui {
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
    void loadArchives();
    void archiveList(QList<ArchivePtr> archives, bool fromRemote = false);
    void deleteArchives(QList<ArchivePtr> archives);
    void loadArchiveStats(ArchivePtr archive);
    void loadArchiveContents(ArchivePtr archive);
    void getOverallStats();
    void repairCache();
    void settingsChanged();
    void purgeArchives();
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void runSetupWizard();
    void stopTasks(bool running, bool queued);
    void jobsList(QMap<QString, JobPtr>);
    void deleteJob(JobPtr job, bool purgeArchives);
    void loadJobs();
    void getTarsnapVersion(QString tarsnapPath);
    void displayNotification(QString message);
    void getTaskInfo();
    void jobAdded(JobPtr job);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent * event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void loadSettings();
    void updateStatusMessage(QString message, QString detail = "");
    void backupTaskUpdate(const TaskStatus &status);
    void archivesDeleted(QList<ArchivePtr> archives);
    void updateLoadingAnimation(bool idle);
    void updateSettingsSummary(quint64 sizeTotal, quint64 sizeCompressed, quint64 sizeUniqueTotal
                               , quint64 sizeUniqueCompressed, quint64 archiveCount);
    void repairCacheStatus(TaskStatus status, QString reason);
    void purgeArchivesStatus(TaskStatus status, QString reason);
    void restoreArchiveStatus(ArchivePtr archive, TaskStatus status, QString reason);
    void setTarsnapVersion(QString versionString);
    void notificationRaise();
    void displayStopTasks(int runningTasks, int queuedTasks);

private slots:
    void notifyArchivesDeleted(QList<ArchivePtr> archives, bool done);
    void updateBackupItemTotals(quint64 count, quint64 size);
    void displayInspectArchive(ArchivePtr archive);
    void updateInspectArchive();
    void commitSettings();
    void validateMachineKeyPath();
    void validateTarsnapPath();
    void validateTarsnapCache();
    void purgeTimerFired();
    void appendToJournalLog(QString msg);
    void appendToConsoleLog(QString msg);
    void browseForBackupItems();
    void appendTimestampCheckBoxToggled(bool checked);
    void backupButtonClicked();
    void accountMachineUseHostnameButtonClicked();
    void accountMachineKeyBrowseButtonClicked();
    void tarsnapPathBrowseButtonClicked();
    void tarsnapCacheBrowseButton();
    void repairCacheButtonClicked();
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

private:
    Ui::MainWindow  *_ui;
    QPixmap          _logo;
    QIcon            _icon;
    QDialog          _aboutWindow;
    QPoint           _windowDragPos;
    QString          _lastTimestamp;
    ArchivePtr       _currentArchiveDetail;
    bool             _useSIPrefixes;
    QTimer           _purgeTimer;
    int              _purgeTimerCount;
    QMessageBox      _purgeCountdownWindow;
    TarsnapAccount   _tarsnapAccount;
    QString          _tarsnapVersion;
};

#endif // MAINWINDOW_H

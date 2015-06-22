#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "taskmanager.h"

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
    void getArchiveList();
    void archiveList(QList<ArchivePtr> archives);
    void deleteArchives(QList<ArchivePtr> archives);
    void loadArchiveStats(ArchivePtr archive);
    void loadArchiveContents(ArchivePtr archive);
    void getOverallStats();
    void repairCache();
    void settingsChanged();
    void purgeArchives();
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void runSetupWizard();
    void stopTask();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent * event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void loadSettings();
    void backupTaskUpdate(const TaskStatus &status);
    void archivesDeleted(QList<ArchivePtr> archives, bool done = true);
    void updateLoadingAnimation(bool idle);
    void updateSettingsSummary(qint64 sizeTotal, qint64 sizeCompressed, qint64 sizeUniqueTotal
                               , qint64 sizeUniqueCompressed, qint64 archiveCount, qreal credit
                               , QString accountStatus);
    void repairCacheStatus(TaskStatus status, QString reason);
    void purgeArchivesStatus(TaskStatus status, QString reason);
    void restoreArchiveStatus(ArchivePtr archive, TaskStatus status, QString reason);

    void updateStatusMessage(QString message, QString detail = "");

private slots:
    void updateBackupItemTotals(qint64 count, qint64 size);
    void displayInspectArchive(ArchivePtr archive);
    void updateInspectArchive();
    void currentPaneChanged(int index);
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
    void purgeArchivesButtonClicked();
    void runSetupWizardClicked();
    void expandJournalButtonToggled(bool checked);
    void downloadsDirBrowseButtonClicked();
    void displayJobDetails(JobPtr job);
    void hideJobDetails();
    void addJobClicked();
    void cancelOperation();
    void stopPurgeCountdown();

private:
    Ui::MainWindow  *_ui;
    QLabel           _tarsnapLogo;
    QMenuBar         _menuBar;
    QMenu            _appMenu;
    QAction          _actionAbout;
    QWidget          _aboutWindow;
    QPoint           _windowDragPos;
    QString          _lastTimestamp;
    ArchivePtr       _currentArchiveDetail;
    bool             _useSIPrefixes;
    QTimer           _purgeTimer;
    int              _purgeTimerCount;
    QMessageBox      _purgeCountdownWindow;
};

#endif // MAINWINDOW_H

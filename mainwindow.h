#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "jobmanager.h"

#include <QWidget>
#include <QLabel>
#include <QMovie>

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void readSettings();

signals:
    void backupNow(BackupJobPtr job);
    void getArchivesList();
    void archivesList(QList<ArchivePtr> archives);
    void deleteArchives(QList<ArchivePtr> archives);
    void loadArchiveStats(ArchivePtr archive);
    void loadArchiveContents(ArchivePtr archive);
    void getOverallStats();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent * event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void backupJobUpdate(BackupJobPtr job);
    void archivesDeleted(QList<ArchivePtr> archives, bool done = true);
    void updateLoadingAnimation(bool idle);
    void updateSettingsSummary(qint64 sizeTotal, qint64 sizeCompressed, qint64 sizeUniqueTotal
                               , qint64 sizeUniqueCompressed, qint64 archiveCount, qreal credit
                               , QString accountStatus);

private slots:
    void updateBackupItemTotals(qint64 count, qint64 size);
    void displayInspectArchive(ArchivePtr archive);
    void updateInspectArchive();
    void updateStatusMessage(QString message, QString detail = "");
    void currentPaneChanged(int index);
    void commitSettings();
    void validateMachineKeyPath();
    void validateTarsnapPath();
    void validateTarsnapCache();

    void on_appendTimestampCheckBox_toggled(bool checked);
    void on_backupListInfoLabel_linkActivated(const QString &link);
    void on_backupButton_clicked();

    void on_accountMachineUseHostnameButton_clicked();

    void on_accountMachineKeyBrowseButton_clicked();

    void on_tarsnapPathBrowseButton_clicked();

    void on_tarsnapCacheBrowseButton_clicked();

private:
    Ui::MainWindow  *_ui;
    QLabel          *_tarsnapLogo;
    QPoint           _windowDragPos;
    QString          _lastTimestamp;
    ArchivePtr       _currentArchiveDetail;
    QMovie           _loadingAnimation;
    bool             _useSIPrefixes;
};

#endif // MAINWINDOW_H

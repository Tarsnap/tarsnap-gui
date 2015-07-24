#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "tarsnapclient.h"
#include "backuptask.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"

#include <QObject>
#include <QThread>
#include <QUrl>
#include <QUuid>
#include <QMap>
#include <QSharedPointer>
#include <QDateTime>
#include <QThreadPool>
#include <QQueue>

class TaskManager : public QObject
{
    Q_OBJECT

public:
    explicit TaskManager(QObject *parent = 0);
    ~TaskManager();

signals:
    // Tarsnap task notifications
    void idle(bool status); // signal if we are working on tasks or not
    void registerMachineStatus(TaskStatus status, QString reason);
    void fsckStatus(TaskStatus status, QString reason);
    void nukeStatus(TaskStatus status, QString reason);
    void archiveList(QList<ArchivePtr> archives, bool fromRemote = false);
    void archivesDeleted(QList<ArchivePtr> archives);
    void overallStats(quint64 sizeTotal, quint64 sizeCompressed, quint64 sizeUniqueTotal
                      , quint64 sizeUniqueCompressed, quint64 archiveCount, qreal credit
                      , QString accountStatus);
    void restoreArchiveStatus(ArchivePtr archive, TaskStatus status, QString reason);
    void jobsList(QMap<QString, JobPtr> jobs);

public slots:
    void loadSettings();

    // Tarsnap tasks
    void registerMachine(QString user, QString password, QString machine
                         ,QString key, QString tarsnapPath, QString cachePath);
    void backupNow(BackupTaskPtr backupTask);
    void loadArchives();
    void getArchiveStats(ArchivePtr archive);
    void getArchiveContents(ArchivePtr archive);
    void deleteArchives(QList<ArchivePtr> archives);
    void getOverallStats();
    void fsck();
    void nuke();
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void runScheduledJobs();
    void stopTasks();
    void loadJobs();
    void deleteJob(JobPtr job, bool purgeArchives);

private slots:
    // post Tarsnap task processing
    void backupTaskFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void backupTaskStarted(QUuid uuid);
    void registerMachineFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void getArchiveListFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void getArchiveStatsFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void getArchiveContentsFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void deleteArchivesFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void overallStatsFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void fsckFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void nukeFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void restoreArchiveFinished(QUuid uuid, QVariant data, int exitCode, QString output);

    // general task management
    void queueTask(TarsnapClient *cli, bool exclusive = false);
    void startTask(TarsnapClient *cli);
    void dequeueTask(QUuid uuid);

private:
    void parseGlobalStats(QString tarsnapOutput);
    void parseArchiveStats(QString tarsnapOutput, bool newArchiveOutput, ArchivePtr archive);
    QString makeTarsnapCommand(QString cmd);

private:
    QString                      _tarsnapDir;
    QString                      _tarsnapCacheDir;
    QString                      _tarsnapKeyFile;
    QThread                      _managerThread; // manager runs on a separate thread
    QMap<QUuid, BackupTaskPtr>   _backupTaskMap; // keeps track of active backup tasks
    QMap<QUuid, ArchivePtr>      _archiveMap;    // keeps track of archives
    QMap<QUuid, TarsnapClient*>  _runningTaskMap; // keeps track of currently executing client tasks
    QQueue<TarsnapClient*>       _taskQueue;     // keeps track of mutually exclusive client tasks pending execution
    QThreadPool                 *_threadPool;
    bool                         _aggressiveNetworking;
    bool                         _preservePathnames;
    bool                         _headless;
    QMap<QString, JobPtr>        _jobMap;
};

#endif // TASKMANAGER_H

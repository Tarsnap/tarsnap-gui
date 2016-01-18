#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "error.h"
#include "backuptask.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "tarsnapclient.h"

#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QQueue>
#include <QSharedPointer>
#include <QThread>
#include <QThreadPool>
#include <QUrl>
#include <QUuid>

class TaskManager : public QObject
{
    Q_OBJECT

public:
    TaskManager();
    ~TaskManager();

    bool headless() const;
    void setHeadless(bool headless);

signals:
    // Tarsnap task notifications
    void idle(bool status); // signal if we are working on tasks or not
    void tarsnapVersion(QString versionString);
    void registerMachineStatus(TaskStatus status, QString reason);
    void fsckStatus(TaskStatus status, QString reason);
    void archiveList(QList<ArchivePtr> archives, bool fromRemote = false);
    void overallStats(quint64 sizeTotal, quint64 sizeCompressed,
                      quint64 sizeUniqueTotal, quint64 sizeUniqueCompressed,
                      quint64 archiveCount);
    void jobsList(QMap<QString, JobPtr> jobs);
    void message(QString msg, QString detail = "");
    void displayNotification(QString message);
    void taskInfo(int runningTasks, int queuedTasks);
    void error(TarsnapError error);

public slots:
    void loadSettings();
    void runScheduledJobs();
    void stopTasks(bool running, bool queued);
    void loadJobs();
    void deleteJob(JobPtr job, bool purgeArchives);
    void loadJobArchives();
    void getTaskInfo();
    void addJob(JobPtr job);

    // Tarsnap tasks
    void getTarsnapVersion(QString tarsnapPath);
    void registerMachine(QString user, QString password, QString machine,
                         QString key, QString tarsnapPath, QString cachePath);
    void backupNow(BackupTaskPtr backupTask);
    void loadArchives();
    void getArchiveStats(ArchivePtr archive);
    void getArchiveContents(ArchivePtr archive);
    void deleteArchives(QList<ArchivePtr> archives);
    void getOverallStats();
    void fsck(bool prune = false);
    void nuke();
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);

private slots:
    // post Tarsnap task processing
    void getTarsnapVersionFinished(QVariant data, int exitCode, QString output);
    void backupTaskFinished(QVariant data, int exitCode, QString output);
    void backupTaskStarted(QVariant data);
    void registerMachineFinished(QVariant data, int exitCode, QString output);
    void getArchiveListFinished(QVariant data, int exitCode, QString output);
    void getArchiveStatsFinished(QVariant data, int exitCode, QString output);
    void getArchiveContentsFinished(QVariant data, int exitCode, QString output);
    void deleteArchivesFinished(QVariant data, int exitCode, QString output);
    void overallStatsFinished(QVariant data, int exitCode, QString output);
    void fsckFinished(QVariant data, int exitCode, QString output);
    void nukeFinished(QVariant data, int exitCode, QString output);
    void restoreArchiveFinished(QVariant data, int exitCode, QString output);
    void notifyBackupTaskUpdate(QUuid uuid, const TaskStatus &status);
    void notifyArchivesDeleted(QList<ArchivePtr> archives, bool done);

    // general task management
    void queueTask(TarsnapClient *cli, bool exclusive = false);
    void startTask(TarsnapClient *cli);
    void dequeueTask();

private:
    void parseError(QString tarsnapOutput);
    void parseGlobalStats(QString tarsnapOutput);
    void parseArchiveStats(QString tarsnapOutput, bool newArchiveOutput,
                           ArchivePtr archive);
    QString makeTarsnapCommand(QString cmd);

private:
    QString _tarsnapDir;
    QString _tarsnapVersion;
    QString _tarsnapCacheDir;
    QString _tarsnapKeyFile;
    QMap<QUuid, BackupTaskPtr> _backupTaskMap;
    QMap<QString, ArchivePtr>  _archiveMap;
    QList<TarsnapClient *>     _runningTasks;
    QQueue<TarsnapClient *>    _taskQueue; // mutually exclusive tasks
    QThreadPool               *_threadPool;
    bool                       _aggressiveNetworking;
    bool                       _preservePathnames;
    bool                       _headless;
    QMap<QString, JobPtr>      _jobMap;
};

#endif // TASKMANAGER_H

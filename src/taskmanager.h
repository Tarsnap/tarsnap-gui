#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "backuptask.h"
#include "error.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "tarsnaptask.h"

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

public slots:
    void runScheduledJobs();
    void stopTasks(bool interrupt, bool running, bool queued);
    void loadArchives();
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
    void getArchives();
    void getArchiveStats(ArchivePtr archive);
    void getArchiveContents(ArchivePtr archive);
    void deleteArchives(QList<ArchivePtr> archives);
    void getOverallStats();
    void fsck(bool prune = false);
    void nuke();
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void getKeyId(QString key);
    void initializeCache();
    void findMatchingArchives(QString jobPrefix);

signals:
    // Tarsnap task notifications
    void idle(bool status); // signal if we are working on tasks or not
    void tarsnapVersion(QString versionString);
    void registerMachineStatus(TaskStatus status, QString reason);
    void fsckStatus(TaskStatus status, QString reason);
    void archiveList(QList<ArchivePtr> archives);
    void addArchive(ArchivePtr archive);
    void overallStats(quint64 sizeTotal, quint64 sizeCompressed,
                      quint64 sizeUniqueTotal, quint64 sizeUniqueCompressed,
                      quint64 archiveCount);
    void jobsList(QMap<QString, JobPtr> jobs);
    void message(QString msg, QString detail = "");
    void displayNotification(QString message);
    void taskInfo(bool backupTaskRunning, int runningTasks, int queuedTasks);
    void error(TarsnapError error);
    void keyId(QString key, int id);
    void matchingArchives(QList<ArchivePtr> archives);

private slots:
    // post Tarsnap task processing
    void getTarsnapVersionFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void backupTaskFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void backupTaskStarted(QVariant data);
    void registerMachineFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void getArchiveListFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void getArchiveStatsFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void getArchiveContentsFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void deleteArchivesFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void overallStatsFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void fsckFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void nukeFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void restoreArchiveFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void notifyBackupTaskUpdate(QUuid uuid, const TaskStatus &status);
    void notifyArchivesDeleted(QList<ArchivePtr> archives, bool done);
    void getKeyIdFinished(QVariant data, int exitCode, QString stdOut, QString stdErr);

    // general task management
    void queueTask(TarsnapTask *task, bool exclusive = false);
    void startTask(TarsnapTask *task);
    void dequeueTask();

private:
    void parseError(QString tarsnapOutput);
    void parseGlobalStats(QString tarsnapOutput);
    void parseArchiveStats(QString tarsnapOutput, bool newArchiveOutput,
                           ArchivePtr archive);
    QString makeTarsnapCommand(QString cmd);
    void    initTarsnapArgs(QStringList &args);

    QMap<QUuid, BackupTaskPtr> _backupTaskMap;
    QMap<QString, ArchivePtr>  _archiveMap;
    QList<TarsnapTask *>       _runningTasks;
    QQueue<TarsnapTask *>      _taskQueue; // mutually exclusive tasks
    QThreadPool               *_threadPool;
    QMap<QString, JobPtr>      _jobMap;
};

#endif // TASKMANAGER_H

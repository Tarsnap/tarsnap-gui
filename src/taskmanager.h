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

/*!
 * \ingroup background-tasks
 * \brief The TaskManager is a QObject which manages background tasks.
 */
class TaskManager : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    TaskManager();
    ~TaskManager();

public slots:
    //! Checks if any scheduled jobs need to run now; if so, adds them to
    //! the queue.  If there are no scheduled jobs, quit the app immediately.
    void runScheduledJobs();
    void stopTasks(bool interrupt, bool running, bool queued);
    //! Load Archives from the PersistentStore.
    void loadArchives();
    //! Load Jobs from the PersistentStore.
    void loadJobs();
    //! Delete a Job, and potentially all associated Archives.
    void deleteJob(JobPtr job, bool purgeArchives);
    //! Load the list of archives belonging to a specific Job (specified
    //! via Qt's `sender()` function call).
    void loadJobArchives();
    //! Emit \ref taskInfo signal with information regarding running and queued
    //! tasks.
    void getTaskInfo();
    //! Add a job to the Jobs list.
    void addJob(JobPtr job);

    // Tarsnap tasks
    //! tarsnap --version
    void getTarsnapVersion(QString tarsnapPath);
    //! tarsnap-keygen.  If the key already exists, run --fsck-prune first.
    void registerMachine(QString user, QString password, QString machine,
                         QString key, QString tarsnapPath, QString cachePath);
    //! tarsnap -c -f \<name\>
    void backupNow(BackupTaskPtr backupTask);
    //! tarsnap --list-archives -vv
    void getArchives();
    //! tarsnap --print-stats -f \<name\>
    void getArchiveStats(ArchivePtr archive);
    //! tarsnap --tv -f \<name\>
    void getArchiveContents(ArchivePtr archive);
    //! tarsnap -d -f \<name\>
    void deleteArchives(QList<ArchivePtr> archives);
    //! tarsnap --print-stats
    void getOverallStats();
    //! tarsnap --fsck or --fsck-prune
    void fsck(bool prune = false);
    //! tarsnap --nuke
    void nuke();
    //! tarsnap -x -f \<name\>, with options.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(QString key_filename);
    //! Ensure that the cache directory has been created.
    void initializeCache();
    void findMatchingArchives(QString jobPrefix);

signals:
    // Tarsnap task notifications
    //! Are there no running tasks?
    void idle(bool status);
    //! Result of tarsnap --version.
    void tarsnapVersion(QString versionString);
    //! Result of tarsnap-keygen.
    void registerMachineStatus(TaskStatus status, QString reason);
    //! A list of all Archives.
    void archiveList(QList<ArchivePtr> archives);
    void addArchive(ArchivePtr archive);
    //! Result of tarsnap --print-stats.
    void overallStats(quint64 sizeTotal, quint64 sizeCompressed,
                      quint64 sizeUniqueTotal, quint64 sizeUniqueCompressed,
                      quint64 archiveCount);
    //! A list of all Jobs.
    void jobsList(QMap<QString, JobPtr> jobs);
    //! A status message should be shown to the user.
    //! \param msg: main text to display.
    //! \param detail: display this text as a mouse-over tooltip.
    void message(QString msg, QString detail = "");
    //! A message that should be shown as a desktop notification (if enabled).
    void displayNotification(QString message);
    //! Information about running tasks.
    //! \param backupTaskRunning: is a backup task currently running?
    //! \param runningTasks: the number of running tasks.
    //! \param queuedTasks: the number of queued tasks
    void taskInfo(bool backupTaskRunning, int runningTasks, int queuedTasks);
    //! The tarsnap CLI experienced an error.
    void error(TarsnapError error);
    //! The tarsnap key ID number.
    //! \param key_filename: the filename.
    //! \param id: the 64-bit unsigned integer representing the key.
    void keyId(QString key_filename, quint64 id);
    void matchingArchives(QList<ArchivePtr> archives);

private slots:
    // post Tarsnap task processing
    void getTarsnapVersionFinished(QVariant data, int exitCode, QString stdOut,
                                   QString stdErr);
    void backupTaskFinished(QVariant data, int exitCode, QString stdOut,
                            QString stdErr);
    void backupTaskStarted(QVariant data);
    void registerMachineFinished(QVariant data, int exitCode, QString stdOut,
                                 QString stdErr);
    void getArchiveListFinished(QVariant data, int exitCode, QString stdOut,
                                QString stdErr);
    void getArchiveStatsFinished(QVariant data, int exitCode, QString stdOut,
                                 QString stdErr);
    void getArchiveContentsFinished(QVariant data, int exitCode, QString stdOut,
                                    QString stdErr);
    void deleteArchivesFinished(QVariant data, int exitCode, QString stdOut,
                                QString stdErr);
    void overallStatsFinished(QVariant data, int exitCode, QString stdOut,
                              QString stdErr);
    void fsckFinished(QVariant data, int exitCode, QString stdOut,
                      QString stdErr);
    void nukeFinished(QVariant data, int exitCode, QString stdOut,
                      QString stdErr);
    void restoreArchiveFinished(QVariant data, int exitCode, QString stdOut,
                                QString stdErr);
    void notifyBackupTaskUpdate(QUuid uuid, const TaskStatus &status);
    void notifyArchivesDeleted(QList<ArchivePtr> archives, bool done);
    void getKeyIdFinished(QVariant data, int exitCode, QString stdOut,
                          QString stdErr);

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
    void initTarsnapArgs(QStringList &args);
    bool waitForOnline();
    void warnNotOnline();

    QMap<QUuid, BackupTaskPtr> _backupTaskMap;
    QMap<QString, ArchivePtr>  _archiveMap;
    QList<TarsnapTask *>  _runningTasks;
    QQueue<TarsnapTask *> _taskQueue; // mutually exclusive tasks
    QThreadPool *         _threadPool;
    QMap<QString, JobPtr> _jobMap;
};

#endif // TASKMANAGER_H

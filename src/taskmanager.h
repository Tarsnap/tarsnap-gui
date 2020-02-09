#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QQueue>
#include <QSharedPointer>
#include <QThread>
#include <QThreadPool>
#include <QUrl>
#include <QUuid>
WARNINGS_ENABLE

#include "backuptask.h"
#include "notification_info.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "tarsnaperror.h"
#include "tarsnaptask.h"

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

#ifdef QT_TESTLIB_LIB
    //! Start running a task.
    void startTask(TarsnapTask *task);
    //! Don't actually run the next task.
    void fakeNextTask();
    //! Block until there's no tasks.
    void waitUntilIdle();
#endif

public slots:
    //! Checks if any scheduled jobs need to run now; if so, adds them to
    //! the queue.  If there are no scheduled jobs, quit the app immediately.
    void runScheduledJobs();
    //! Stop / interrupt / terminate / dequeue tasks.
    //! \param interrupt Kill the first task.  \warning MacOS X only.  (?)
    //! \param running Stop all running tasks.
    //! \param queued Remove all tasks from the queue.
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
    //! \anchor tarsnapVersionFind
    void tarsnapVersionFind();
    //! tarsnap-keygen.  If the key already exists, run --fsck-prune first.
    void registerMachineDo(const QString &password,
                           const bool     useExistingKeyfile);
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
    void fsck(bool prune);
    //! tarsnap --nuke
    void nuke();
    //! tarsnap -x -f \<name\>, with options.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(const QString &key_filename);
    //! Search for all matching Archive objects which were created by a Job.
    //! \param jobPrefix prefix of the Archive names to match.
    void findMatchingArchives(const QString &jobPrefix);

signals:
    // Tarsnap task notifications
    //! Are there no running tasks?
    void idle(bool status);
    //! Give number of tasks.
    void numTasks(int runningTasks, int queuedTasks);
    //! Result of tarsnap --version.
    //! \anchor tarsnapVersionFound
    void tarsnapVersionFound(TaskStatus status, const QString &versionString);
    //! Result of tarsnap-keygen.
    void registerMachineDone(TaskStatus status, const QString &reason);
    //! A list of all Archive objects.
    void archiveList(QList<ArchivePtr> archives);
    //! A new Archive was created by \ref backupNow or discovered via
    //! \ref getArchives.
    void addArchive(ArchivePtr archive);
    //! Result of tarsnap --print-stats.
    void overallStats(quint64 sizeTotal, quint64 sizeCompressed,
                      quint64 sizeUniqueTotal, quint64 sizeUniqueCompressed,
                      quint64 archiveCount);
    //! A list of all Jobs.
    void jobList(QMap<QString, JobPtr> jobs);
    //! A status message should be shown to the user.
    //! \param msg main text to display.
    //! \param detail display this text as a mouse-over tooltip.
    void message(const QString &msg, const QString &detail = "");
    //! A message that should be shown as a desktop notification (if enabled).
    void displayNotification(const QString &message, enum message_type type,
                             const QString &data);
    //! Information about running tasks.
    //! \param backupTaskRunning is a backup task currently running?
    //! \param runningTasks the number of running tasks.
    //! \param queuedTasks the number of queued tasks
    void taskInfo(bool backupTaskRunning, int runningTasks, int queuedTasks);
    //! The tarsnap CLI experienced an error.
    void error(TarsnapError error);
    //! The tarsnap key ID number.
    //! \param key_filename the filename.
    //! \param id the 64-bit unsigned integer representing the key.
    void keyId(const QString &key_filename, quint64 id);
    //! Archives which match the previously-given search string.
    void matchingArchives(QList<ArchivePtr> archives);

private slots:
    // post Tarsnap task processing
    void getTarsnapVersionFinished(QVariant data, int exitCode,
                                   const QString &stdOut,
                                   const QString &stdErr);
    void backupTaskFinished(QVariant data, int exitCode, const QString &stdOut,
                            const QString &stdErr);
    void backupTaskStarted(QVariant data);
    void registerMachineFinished(QVariant data, int exitCode,
                                 const QString &stdOut, const QString &stdErr);
    void getArchiveListFinished(QVariant data, int exitCode,
                                const QString &stdOut, const QString &stdErr);
    void getArchiveStatsFinished(QVariant data, int exitCode,
                                 const QString &stdOut, const QString &stdErr);
    void getArchiveContentsFinished(QVariant data, int exitCode,
                                    const QString &stdOut,
                                    const QString &stdErr);
    void deleteArchivesFinished(QVariant data, int exitCode,
                                const QString &stdOut, const QString &stdErr);
    void overallStatsFinished(QVariant data, int exitCode,
                              const QString &stdOut, const QString &stdErr);
    void fsckFinished(QVariant data, int exitCode, const QString &stdOut,
                      const QString &stdErr);
    void nukeFinished(QVariant data, int exitCode, const QString &stdOut,
                      const QString &stdErr);
    void restoreArchiveFinished(QVariant data, int exitCode,
                                const QString &stdOut, const QString &stdErr);
    void notifyBackupTaskUpdate(QUuid uuid, const TaskStatus &status);
    void notifyArchivesDeleted(QList<ArchivePtr> archives, bool done);
    void getKeyIdFinished(QVariant data, int exitCode, const QString &stdOut,
                          const QString &stdErr);

    // general task management
    void queueTask(TarsnapTask *task, bool exclusive = false);
    void dequeueTask();
#ifndef QT_TESTLIB_LIB
    void startTask(TarsnapTask *task);
#endif

private:
    void parseError(const QString &tarsnapOutput);
    void parseGlobalStats(const QString &tarsnapOutput);
    void parseArchiveStats(const QString &tarsnapOutput, bool newArchiveOutput,
                           ArchivePtr archive);
    QString makeTarsnapCommand(const QString &cmd);
    void    initTarsnapArgs(QStringList &args);
    bool    waitForOnline();
    void    warnNotOnline();

    QMap<QUuid, BackupTaskPtr> _backupTaskMap;
    QMap<QString, ArchivePtr>  _archiveMap;
    QList<TarsnapTask *>       _runningTasks;
    QQueue<TarsnapTask *>      _taskQueue; // mutually exclusive tasks
    QThreadPool *              _threadPool;
    QMap<QString, JobPtr>      _jobMap;

#ifdef QT_TESTLIB_LIB
    bool _fakeNextTask;
#endif
};

#endif // TASKMANAGER_H

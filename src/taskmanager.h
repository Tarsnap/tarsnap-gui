#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QVariant>
WARNINGS_ENABLE

#include "messages/archiveptr.h"
#include "messages/backuptaskdataptr.h"
#include "messages/jobptr.h"
#include "messages/notification_info.h"
#include "messages/tarsnaperror.h"
#include "messages/taskstatus.h"

/* Forward declaration(s). */
class BackendData;
class BaseTask;
class TaskQueuer;
struct ArchiveRestoreOptions;

/*!
 * \ingroup background-tasks
 * \brief The TaskManager is a QObject which manages background tasks.
 *
 * The actual task queues are handled by \ref TaskQueuer, which is
 * internal to this object.
 */
class TaskManager : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    TaskManager();
    ~TaskManager() override;

#ifdef QT_TESTLIB_LIB
    //! Prepare a task, and start running it if there's no queue.
    void queueTask(BaseTask *task, bool exclusive = false,
                   bool isBackup = false);
    //! Don't actually run the next task.
    void fakeNextTask();
    //! Block until there's no tasks.
    void waitUntilIdle();
#endif

#ifdef QT_TESTLIB_LIB
    //! Queue a task which sleeps for seconds.
    void sleepSeconds(int seconds, bool exclusive);
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
    void deleteJob(const JobPtr &job, bool purgeArchives);
    //! Add a job to the Jobs list.
    void addJob(const JobPtr &job);

    // Tarsnap tasks
    //! tarsnap --version
    //! \anchor tarsnapVersionFind
    void tarsnapVersionFind();
    //! tarsnap-keygen.  If the key already exists, run --fsck-prune first.
    void registerMachineDo(const QString &password, const QString &machine,
                           const bool useExistingKeyfile);
    //! tarsnap -c -f \<name\>
    void backupNow(const BackupTaskDataPtr &backupTaskData);
    //! tarsnap --list-archives -vv
    void getArchives();
    //! tarsnap --print-stats -f \<name\>
    void getArchiveStats(const ArchivePtr &archive);
    //! tarsnap --tv -f \<name\>
    void getArchiveContents(const ArchivePtr &archive);
    //! tarsnap -d -f \<name\>
    void deleteArchives(const QList<ArchivePtr> &archives);
    //! tarsnap --print-stats
    void getOverallStats();
    //! tarsnap --fsck or --fsck-prune
    void fsck(bool prune);
    //! tarsnap --nuke
    void nuke();
    //! tarsnap -x -f \<name\>, with options.
    void restoreArchive(const ArchivePtr            &archive,
                        const ArchiveRestoreOptions &options);
    //! tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(const QString &key_filename);
    //! Search for all matching Archive objects which were created by a Job.
    //! \param jobPrefix prefix of the Archive names to match.
    void findMatchingArchives(const QString &jobPrefix);

    //! Prepare a task, and start it when there's an available thread.
    void queueGuiTask(BaseTask *task);

    //! Request cancelling a GUI task.
    void cancelGuiTask(BaseTask *task, const QUuid &uuid);

signals:
    // Tarsnap task notifications
    //! Give number of tasks.
    //! \param backupRunning is a backup task currently running?
    //! \param runningTasks the number of running tasks.
    //! \param queuedTasks the number of queued tasks
    void numTasks(bool backupRunning, int runningTasks, int queuedTasks);
    //! Result of tarsnap --version.
    //! \anchor tarsnapVersionFound
    void tarsnapVersionFound(TaskStatus status, const QString &versionString);
    //! Intermediate progress of tarsnap --fsck.
    void registerMachineProgress(const QString &stdOut);
    //! Result of tarsnap-keygen.
    void registerMachineDone(TaskStatus status, const QString &reason);
    //! A list of all Archive objects.
    void archiveList(QList<ArchivePtr> archives);
    //! A new Archive was created by \ref backupNow or discovered via
    //! \ref getArchives.
    void archiveAdded(ArchivePtr archive);
    //! Result of tarsnap --print-stats.
    void overallStats(quint64 sizeTotal, quint64 sizeCompressed,
                      quint64 sizeUniqueTotal, quint64 sizeUniqueCompressed,
                      quint64 archiveCount);
    //! A list of all Jobs.
    void jobList(QMap<QString, JobPtr> jobs);
    //! A status message should be shown to the user.
    //! \param msg main text to display.
    void message(const QString &msg);
    //! A message that should be shown as a desktop notification (if enabled).
    void displayNotification(const QString &message, enum message_type type,
                             const QString &data);
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
    void getTarsnapVersionFinished(const QVariant &data, int exitCode,
                                   const QString &stdOut,
                                   const QString &stdErr);
    void backupTaskFinished(const QVariant &data, int exitCode,
                            const QString &stdOut, const QString &stdErr);
    void backupTaskStarted(const QVariant &data);
    void registerMachineFinished(const QVariant &data, int exitCode,
                                 const QString &stdOut, const QString &stdErr);
    void getArchiveListFinished(const QVariant &data, int exitCode,
                                const QString &stdOut, const QString &stdErr);
    void getArchiveStatsFinished(const QVariant &data, int exitCode,
                                 const QString &stdOut, const QString &stdErr);
    void getArchiveContentsFinished(const QVariant &data, int exitCode,
                                    const QString &stdOut,
                                    const QString &stdErr);
    void deleteArchivesFinished(const QVariant &data, int exitCode,
                                const QString &stdOut, const QString &stdErr);
    void overallStatsFinished(const QVariant &data, int exitCode,
                              const QString &stdOut, const QString &stdErr);
    void fsckFinished(const QVariant &data, int exitCode, const QString &stdOut,
                      const QString &stdErr);
    void nukeFinished(const QVariant &data, int exitCode, const QString &stdOut,
                      const QString &stdErr);
    void restoreArchiveFinished(const QVariant &data, int exitCode,
                                const QString &stdOut, const QString &stdErr);
    void notifyBackupTaskUpdate(const BackupTaskDataPtr &backupTaskData,
                                const TaskStatus        &status);
    void notifyArchivesDeleted(QList<ArchivePtr> archives, bool done);
    void getKeyIdFinished(const QVariant &data, int exitCode,
                          const QString &stdOut, const QString &stdErr);

private:
    void parseError(const QString &tarsnapOutput);
    void parseGlobalStats(const QString &tarsnapOutput);
    void parseArchiveStats(const QString &tarsnapOutput, bool newArchiveOutput,
                           const ArchivePtr &archive);
    bool waitForOnline();
    void warnNotOnline();

    TaskQueuer *_tq;

    BackendData *_bd;
};

#endif // TASKMANAGER_H

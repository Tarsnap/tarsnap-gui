#ifndef TASKQUEUER_H
#define TASKQUEUER_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QQueue>
#include <QThreadPool>
#include <QUuid>
WARNINGS_ENABLE

#include "cmdlinetask.h"

/*!
 * \ingroup background-tasks
 * \brief The TaskQueuer is a QObject which manages background tasks.
 */
class TaskQueuer : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    TaskQueuer();
    ~TaskQueuer();

#ifdef QT_TESTLIB_LIB
    //! Prepare a task, and start running it if there's no queue.
    void queueTask(CmdlineTask *task, bool exclusive = false,
                   bool isBackup = false);
    //! Don't actually run the next task.
    void fakeNextTask();
    //! Block until there's no tasks.
    void waitUntilIdle();
#endif

public slots:
    //! Stop / interrupt / terminate / dequeue tasks.
    //! \param interrupt Kill the first task.  \warning MacOS X only.  (?)
    //! \param running Stop all running tasks.
    //! \param queued Remove all tasks from the queue.
    void stopTasks(bool interrupt, bool running, bool queued);
    //! Emit \ref taskInfo signal with information regarding running and queued
    //! tasks.
    void getTaskInfo();

signals:
    //! Give number of tasks.
    void numTasks(bool backupRunning, int runningTasks, int queuedTasks);
    //! A status message should be shown to the user.
    //! \param msg main text to display.
    //! \param detail display this text as a mouse-over tooltip.
    void message(const QString &msg, const QString &detail = "");
    //! Information about running tasks.
    //! \param backupTaskRunning is a backup task currently running?
    //! \param runningTasks the number of running tasks.
    //! \param queuedTasks the number of queued tasks
    void taskInfo(bool backupTaskRunning, int runningTasks, int queuedTasks);

private slots:
#ifndef QT_TESTLIB_LIB
    void queueTask(CmdlineTask *task, bool exclusive = false,
                   bool isBackup = false);
#endif
    void dequeueTask();
    void startTask(CmdlineTask *task);

private:
    bool isBackupTaskRunning();

    QList<CmdlineTask *>  _runningTasks;
    QQueue<CmdlineTask *> _taskQueue; // mutually exclusive tasks
    QThreadPool *         _threadPool;

    // Keep track of which CmdlineTasks are backups.
    QList<QUuid> _backupUuidList;

#ifdef QT_TESTLIB_LIB
    bool _fakeNextTask;
#endif
};

#endif // !TASKQUEUER_H

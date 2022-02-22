#ifndef TASKQUEUER_H
#define TASKQUEUER_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QObject>
#include <QQueue>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
class BaseTask;
class QThreadPool;
class QUuid;
struct TaskMeta;

/*!
 * \ingroup background-tasks
 * \brief The TaskQueuer is a QObject which manages the background task queues.
 *
 * It is used internally by \ref TaskManager.
 */
class TaskQueuer : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    TaskQueuer();
    ~TaskQueuer() override;

    //! Prepare a task, and start running it if there's no queue.
    void queueTask(BaseTask *task, bool exclusive = false,
                   bool isBackup = false);

    //! Stop / interrupt / terminate / dequeue tasks.
    //! \param interrupt Kill the first task.  \warning MacOS X only.  (?)
    //! \param running Stop all running tasks.
    //! \param queued Remove all tasks from the queue.
    void stopTasks(bool interrupt, bool running, bool queued);

    //! Request cancelling a task.
    void cancelTask(BaseTask *task, const QUuid &uuid);

#ifdef QT_TESTLIB_LIB
    //! Don't actually run the next task.
    void fakeNextTask();
    //! Block until there's no tasks.
    void waitUntilIdle();
#endif

signals:
    //! Give number of tasks.
    //! \param backupRunning is a backup task currently running?
    //! \param runningTasks the number of running tasks.
    //! \param queuedTasks the number of queued tasks
    void numTasks(bool backupRunning, int runningTasks, int queuedTasks);
    //! A status message should be shown to the user.
    //! \param msg main text to display.
    //! \param detail display this text as a mouse-over tooltip.
    void message(const QString &msg, const QString &detail = "");

private slots:
    void dequeueTask();

private:
    void startTask();
    void startTasks();
    bool isExclusiveTaskRunning();
    bool isBackupTaskRunning();
    void updateTaskNumbers();

    QList<TaskMeta *>  _runningTasks;
    QQueue<TaskMeta *> _taskQueue;
    QThreadPool       *_threadPool;

#ifdef QT_TESTLIB_LIB
    bool _fakeNextTask;
#endif
};

#endif // !TASKQUEUER_H

#include "taskqueuer.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QEventLoop>
#include <QThreadPool>
#include <QUuid>
WARNINGS_ENABLE

#include "basetask.h"
#include "cmdlinetask.h"

/*! Track info about tasks. */
struct TaskMeta
{
    /*! Actual task. */
    BaseTask *task;
    /*! Does this task need to be the only one happening? */
    bool isExclusive;
    /*! This is a "create archive" task? */
    bool isBackup;
};

TaskQueuer::TaskQueuer() : _threadPool(QThreadPool::globalInstance())
{
#ifdef QT_TESTLIB_LIB
    _fakeNextTask = false;
#endif
}

TaskQueuer::~TaskQueuer()
{
    // Wait up to 1 second to finish any background tasks
    _threadPool->waitForDone(1000);
    // Wait up to 1 second to delete objects scheduled with ->deleteLater()
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void TaskQueuer::stopTasks(bool interrupt, bool running, bool queued)
{
    // Clear the queue first, to avoid starting a queued task
    // after already clearing the running task(s).
    if(queued)
    {
        while(!_taskQueue.isEmpty())
        {
            TaskMeta *tm   = _taskQueue.dequeue();
            BaseTask *task = tm->task;
            if(task)
            {
                task->canceled();
                task->deleteLater();
            }
        }
        emit message("Cleared queued tasks.");
    }

    // Deal with a running backup.
    if(interrupt)
    {
        // Sending a SIGQUIT will cause the tarsnap binary to
        // create a checkpoint.  Non-tarsnap binaries should be
        // receive a BaseTask::stop() instead of a SIGQUIT.
        if(!_runningTasks.isEmpty())
        {
            TaskMeta *tm = _runningTasks.first();
            Q_ASSERT(tm->isBackup == true);
            CmdlineTask *backupTask = qobject_cast<CmdlineTask *>(tm->task);
            Q_ASSERT(backupTask != nullptr);
            backupTask->sigquit();
        }
        emit message("Interrupting current backup.");
    }

    // Stop running tasks.
    if(running)
    {
        for(TaskMeta *tm : _runningTasks)
        {
            BaseTask *task = tm->task;
            if(task)
                task->stop();
        }
        emit message("Stopped running tasks.");
    }
}

void TaskQueuer::queueTask(BaseTask *task, bool exclusive, bool isBackup)
{
    // Sanity check.
    Q_ASSERT(task != nullptr);

    // Create & initialize the TaskMeta object.
    TaskMeta *tm    = new TaskMeta;
    tm->task        = task;
    tm->isExclusive = exclusive;
    tm->isBackup    = isBackup;

    // Add to the queue and trigger starting a new task.
    _taskQueue.enqueue(tm);
    startTasks();
}

void TaskQueuer::startTasks()
{
    while(!_taskQueue.isEmpty() && !isExclusiveTaskRunning())
    {
        // Bail from loop if the next task requires exclusive running.
        if(!_runningTasks.isEmpty() && _taskQueue.head()->isExclusive)
            break;

        // Bail from loop if we've reached the maximum number of threads.
        if(_runningTasks.count() >= _threadPool->maxThreadCount())
            break;

        startTask();
    }

    // Send the updated task numbers.
    updateTaskNumbers();
}

void TaskQueuer::startTask()
{
    // Bail if there's nothing to do.
    if(_taskQueue.isEmpty())
        return;

    // Check for exclusive.
    if(isExclusiveTaskRunning())
        return;

    // Get a new task.
    TaskMeta *tm = _taskQueue.dequeue();

    // Set up the task ending.
    BaseTask *task = tm->task;
    connect(task, &BaseTask::dequeue, this, &TaskQueuer::dequeueTask);
    task->setAutoDelete(false);

    // Record this thread as "running", even though it hasn't actually
    // started yet.  QThreadPool::start() is non-blocking, and in fact
    // explicitly states that a QRunnable can be added to an internal
    // run queue if it's exceeded QThreadPoll::maxThreadCount().
    //
    // However, for the purpose of this TaskQueuer, the task should not
    // be recorded in our _taskQueue (because we've just dequeued()'d it).
    // The "strictly correct" solution would be to add a
    // _waitingForStart queue, and move items out of that queue when the
    // relevant BaseTask::started signal was emitted.  At the moment,
    // I don't think that step is necessary, but I might need to revisit
    // that decision later.
    _runningTasks.append(tm);

    // Start the task.
#ifdef QT_TESTLIB_LIB
    if(_fakeNextTask)
        task->fake();
#endif
    _threadPool->start(task);
}

void TaskQueuer::cancelTask(BaseTask *task, const QUuid &uuid)
{
    // Remove from the queue.
    for(TaskMeta *tm : _taskQueue)
    {
        // We need to use the uuid to verify that the task pointer is still
        // valid -- theoretically, we could have completed a task, deleted
        // the pointer, then allocated a new task at the same address.
        // However, it's statistically impossible for the new task to have
        // the same uuid as the previous one.
        if((tm->task == task) && (tm->task->uuid() == uuid))
        {
            _taskQueue.removeOne(tm);
            tm->task->canceled();
            tm->task->deleteLater();
        }
    }
    // Stop if it's running
    for(TaskMeta *tm : _runningTasks)
    {
        // Again, use the uuid to verify that it's the right pointer.
        if((tm->task == task) && (tm->task->uuid() == uuid))
            tm->task->stop();
    }
}

void TaskQueuer::dequeueTask()
{
    // Get the task.
    BaseTask *task = qobject_cast<BaseTask *>(sender());

    // Sanity check.
    if(task == nullptr)
        return;

    // Clean up task.
    for(TaskMeta *tm : _runningTasks)
    {
        if(tm->task == task)
        {
            _runningTasks.removeOne(tm);
            delete tm;
            break;
        }
    }
    task->deleteLater();

    // Start another task(s) if applicable.
    startTasks();
}

bool TaskQueuer::isExclusiveTaskRunning()
{
    for(TaskMeta *tm : _runningTasks)
    {
        if(tm->isExclusive)
            return true;
    }
    return false;
}

bool TaskQueuer::isBackupTaskRunning()
{
    for(TaskMeta *tm : _runningTasks)
    {
        if(tm->isBackup)
            return true;
    }
    return false;
}

void TaskQueuer::updateTaskNumbers()
{
    bool backupTaskRunning = isBackupTaskRunning();
    emit numTasks(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
}

#ifdef QT_TESTLIB_LIB
void TaskQueuer::fakeNextTask()
{
    _fakeNextTask = true;
}

void TaskQueuer::waitUntilIdle()
{
    while(!(_taskQueue.isEmpty() && _runningTasks.isEmpty()))
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
#endif

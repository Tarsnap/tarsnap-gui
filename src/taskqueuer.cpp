#include "taskqueuer.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QEventLoop>
#include <QVariant>
WARNINGS_ENABLE

#include "cmdlinetask.h"

struct TaskMeta
{
    CmdlineTask *task;
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
            TaskMeta *   tm   = _taskQueue.dequeue();
            CmdlineTask *task = tm->task;
            if(task)
            {
                task->emitCanceled();
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
        // receive a CmdlineTask::stop() instead of a SIGQUIT.
        if(!_runningTasks.isEmpty())
            _runningTasks.first()->task->sigquit();
        emit message("Interrupting current backup.");
    }

    // Stop running tasks.
    if(running)
    {
        for(TaskMeta *tm : _runningTasks)
        {
            CmdlineTask *task = tm->task;
            if(task)
                task->stop();
        }
        emit message("Stopped running tasks.");
    }
}

void TaskQueuer::queueTask(CmdlineTask *task, bool exclusive, bool isBackup)
{
    // Sanity check.
    Q_ASSERT(task != nullptr);

    // Add to list of backup tasks (if applicable).
    if(isBackup)
        _backupUuidList.append(task->uuid());

    // Create & initialize the TaskMeta object.
    TaskMeta *tm = new TaskMeta;
    tm->task     = task;

    // Add to the queue and trigger starting a new task.
    if(exclusive && !_runningTasks.isEmpty())
        _taskQueue.enqueue(tm);
    else
        startTask(tm);
}

void TaskQueuer::startTask(TaskMeta *tm)
{
    // Ensure that we have a task, or bail.
    if(tm == nullptr)
    {
        if(!_taskQueue.isEmpty())
            tm = _taskQueue.dequeue();
        else
            return;
    }

    // Set up the task ending.
    CmdlineTask *task = tm->task;
    connect(task, &CmdlineTask::dequeue, this, &TaskQueuer::dequeueTask);
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
    // relevant CmdlineTask::started signal was emitted.  At the moment,
    // I don't think that step is necessary, but I might need to revisit
    // that decision later.
    _runningTasks.append(tm);

    // Start the task.
#ifdef QT_TESTLIB_LIB
    if(_fakeNextTask)
        task->fake();
#endif
    _threadPool->start(task);

    // Update the task numbers.
    bool backupTaskRunning = isBackupTaskRunning();
    emit numTasks(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
}

void TaskQueuer::dequeueTask()
{
    // Get the task.
    CmdlineTask *task = qobject_cast<CmdlineTask *>(sender());

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
    _backupUuidList.removeAll(task->uuid());
    task->deleteLater();

    // Start another task.
    if(_runningTasks.isEmpty())
        startTask(nullptr);

    // Update the task numbers.
    bool backupTaskRunning = isBackupTaskRunning();
    emit numTasks(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
}

bool TaskQueuer::isBackupTaskRunning()
{
    if(!_runningTasks.isEmpty() && !_backupUuidList.isEmpty())
    {
        for(TaskMeta *tm : _runningTasks)
        {
            CmdlineTask *task = tm->task;
            if(task && _backupUuidList.contains(task->uuid()))
            {
                return true;
            }
        }
    }
    return false;
}

void TaskQueuer::getTaskInfo()
{
    bool backupTaskRunning = isBackupTaskRunning();
    emit taskInfo(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
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

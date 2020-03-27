#include "taskqueuer.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QEventLoop>
#include <QVariant>
WARNINGS_ENABLE

#include "cmdlinetask.h"

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
            CmdlineTask *task = _taskQueue.dequeue();
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
            _runningTasks.first()->sigquit();
        emit message("Interrupting current backup.");
    }

    // Stop running tasks.
    if(running)
    {
        for(CmdlineTask *task : _runningTasks)
        {
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

    // Add to the queue and trigger starting a new task.
    if(exclusive && !_runningTasks.isEmpty())
        _taskQueue.enqueue(task);
    else
        startTask(task);
}

void TaskQueuer::startTask(CmdlineTask *task)
{
    // Ensure that we have a task, or bail.
    if(task == nullptr)
    {
        if(!_taskQueue.isEmpty())
            task = _taskQueue.dequeue();
        else
            return;
    }

    // Set up the task ending.
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
    _runningTasks.append(task);

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
    for(CmdlineTask *t : _runningTasks)
    {
        if(t == task)
        {
            _runningTasks.removeOne(t);
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
        for(CmdlineTask *task : _runningTasks)
        {
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

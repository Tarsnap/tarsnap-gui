#ifndef TASKSTATUS_H
#define TASKSTATUS_H

#include <QMetaType>

/*!
 * \ingroup background-tasks
 * \brief Status which can occur from running a TarsnapTask.
 */
enum TaskStatus
{
    Initialized,
    Queued,
    Running,
    Completed,
    Failed,
    Paused
};

Q_DECLARE_METATYPE(TaskStatus);

#endif

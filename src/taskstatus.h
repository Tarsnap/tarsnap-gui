#ifndef TASKSTATUS_H
#define TASKSTATUS_H

#include <QMetaType>

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

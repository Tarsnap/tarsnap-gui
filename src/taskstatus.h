#ifndef TASKSTATUS_H
#define TASKSTATUS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QMetaType>
WARNINGS_ENABLE

/*!
 * \ingroup background-tasks
 * \brief Status which can occur from running a CmdlineTask.
 */
enum TaskStatus
{
    Initialized,
    Queued,
    Running,
    Completed,
    Failed,
    VersionTooLow, /* For 'tarsnap --version' only. */
    Paused
};

Q_DECLARE_METATYPE(TaskStatus)

#endif /* !TASKSTATUS_H */

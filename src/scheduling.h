#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

enum schedulestatus
{
    SCHEDULE_OK,
    SCHEDULE_ERROR,
    SCHEDULE_NEED_INFO,
    SCHEDULE_NOTHING_HAPPENED
};

//! Information about a schedule function.
struct scheduleinfo
{
    //! Did the operation succeed?
    schedulestatus status;
    //! A message clarifying the status.
    QString message;
    //! Extra string used in special circumstances; see code.
    QString extra;
};

struct scheduleinfo launchdEnable();
struct scheduleinfo launchdDisable();
struct scheduleinfo cronEnable();
struct scheduleinfo cronEnable_p2(const QString &cronBlock,
                                  const QString &existingCrontab);
struct scheduleinfo cronDisable();
struct scheduleinfo cronDisable_p2(const QString &linesToRemove,
                                   const QString &existingCrontab);

//! Check and update (if necessary) the path to the Tarsnap-GUI binary
struct scheduleinfo correctedSchedulingPath();

#endif

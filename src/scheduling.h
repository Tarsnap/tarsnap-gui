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

struct scheduleinfo
{
    schedulestatus status;
    QString        message;
    QString        extra;
};

struct scheduleinfo launchdEnable();
struct scheduleinfo launchdDisable();
struct scheduleinfo cronEnable();
struct scheduleinfo cronEnable_p2(QString cronBlock, QString currentCrontab);
struct scheduleinfo cronDisable();
struct scheduleinfo cronDisable_p2(QString linesToRemove,
                                   QString currentCrontab);

//! Check and update (if necessary) the path to the Tarsnap-GUI binary
struct scheduleinfo correctedSchedulingPath();

#endif

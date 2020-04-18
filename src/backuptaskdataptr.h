#ifndef BACKUPTASKDATAPTR_H
#define BACKUPTASKDATAPTR_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QSharedPointer>
WARNINGS_ENABLE

class BackupTaskData;

typedef QSharedPointer<BackupTaskData> BackupTaskDataPtr;

#endif /* !BACKUPTASKDATAPTR_H */

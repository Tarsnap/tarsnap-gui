#ifndef JOBPTR_H
#define JOBPTR_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QSharedPointer>
WARNINGS_ENABLE

/* Forward declaration(s). */
class Job;
typedef QSharedPointer<Job> JobPtr;

#endif /* !JOBPTR_H */

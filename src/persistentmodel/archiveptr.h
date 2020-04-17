#ifndef ARCHIVEPTR_H
#define ARCHIVEPTR_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QSharedPointer>
WARNINGS_ENABLE

/* Forward declaration(s). */
class Archive;
typedef QSharedPointer<Archive> ArchivePtr;

#endif /* !ARCHIVEPTR_H */

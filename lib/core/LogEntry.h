#ifndef LOGENTRY_H
#define LOGENTRY_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QMetaObject>
#include <QString>
WARNINGS_ENABLE

//! Info to add to the log.
struct LogEntry
{
    //! Time of the entry.
    QDateTime timestamp;
    //! Text to add.
    QString message;
};

Q_DECLARE_METATYPE(LogEntry)

#endif // !LOGENTRY_H

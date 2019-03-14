#ifndef DEBUG_H
#define DEBUG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDebug>
WARNINGS_ENABLE

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define DEBUG qDebug().noquote()
#else
#define DEBUG qDebug()
#endif

#endif // DEBUG_H

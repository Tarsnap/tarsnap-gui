#ifndef UTILS_H
#define UTILS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QFileInfoList>
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define APPDATA QStandardPaths::AppLocalDataLocation
#else
#define APPDATA QStandardPaths::DataLocation
#endif

namespace Utils
{

} // namespace Utils

#endif // UTILS_H

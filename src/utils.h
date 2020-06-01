#ifndef UTILS_H
#define UTILS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QFileInfoList>
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

#define DEFAULT_TRAVERSE_MOUNT true
#define DEFAULT_FOLLOW_SYMLINKS false
#define DEFAULT_DRY_RUN false
#define DEFAULT_SKIP_NODUMP false
#define DEFAULT_SKIP_FILES_SIZE 0

#define DEFAULT_SKIP_SYSTEM_ENABLED false
#if defined Q_OS_OSX
#define DEFAULT_SKIP_SYSTEM_FILES                                              \
    ".DS_Store:.localized:.fseventsd:.Spotlight-V100:._.Trashes:.Trashes"
#elif defined Q_OS_WIN
#define DEFAULT_SKIP_SYSTEM_FILES                                              \
    "$RECYCLE.BIN:System Volume Information:Thumbs.db"
#elif defined Q_OS_LINUX
#define DEFAULT_SKIP_SYSTEM_FILES ".lost+found"
#else
#define DEFAULT_SKIP_SYSTEM_FILES ""
#endif

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define APPDATA QStandardPaths::AppLocalDataLocation
#else
#define APPDATA QStandardPaths::DataLocation
#endif

namespace Utils
{

} // namespace Utils

#endif // UTILS_H

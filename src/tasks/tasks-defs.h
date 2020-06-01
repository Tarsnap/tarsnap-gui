#ifndef TASKS_DEFS_H
#define TASKS_DEFS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QStandardPaths>
WARNINGS_ENABLE

/**
 * \file tasks-defs.h
 * \brief Useful definitions for Tarsnap tasks.
 */

/** @{ Tarsnap binaries */
#define CMD_TARSNAP "tarsnap"
#define CMD_TARSNAPKEYGEN "tarsnap-keygen"
#define CMD_TARSNAPKEYMGMT "tarsnap-keymgmt"
#define TARSNAP_MIN_VERSION "1.0.39"
/** @} */

/** @{ Checking that we can access the Tarsnap server */
#define NETWORK_HOST "v1-0-0-server.tarsnap.com"
#define NETWORK_PORT 9279
#define NETWORK_UP_SLEEP 60
#define NETWORK_UP_ATTEMPTS 3
/** @} */

/** @{ Default behaviour for creating Tarsnap archives */
#define DEFAULT_AGGRESSIVE_NETWORKING false
#define DEFAULT_NO_DEFAULT_CONFIG true
#define DEFAULT_PRESERVE_PATHNAMES true
#define DEFAULT_TRAVERSE_MOUNT true
#define DEFAULT_FOLLOW_SYMLINKS false
#define DEFAULT_DRY_RUN false

/* Default behaviour for skipping files */
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
/** @} */

/** @{ Default behaviour for the GUI */
#define DEFAULT_DOWNLOADS                                                      \
    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define APPDATA QStandardPaths::AppLocalDataLocation
#else
#define APPDATA QStandardPaths::DataLocation
#endif
/** @} */

#endif /* !TASKS_DEFS_H */

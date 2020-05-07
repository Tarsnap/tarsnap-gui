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
/** @} */

/** @{ Default behaviour for the GUI */
#define DEFAULT_DOWNLOADS                                                      \
    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)

#endif /* !TASKS_DEFS_H */

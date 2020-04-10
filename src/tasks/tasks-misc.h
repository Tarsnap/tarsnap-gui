#ifndef TASKS_MISC_H
#define TASKS_MISC_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
class CmdlineTask;

/**
 * \file tasks-misc.h
 * \brief These functions are used for miscellaneous tasks which
 * are not part of \ref tasks-setup.h or \ref tasks-tarsnap.h .
 *
 * They:
 * - should not have any side effects.
 * - should read information from TSettings (if possible), instead
 *   of using parameters.
 */

/**
 * \brief Create a task for: `tarsnap-keymgmt --print-key-id KEYFILENAME`
 */
CmdlineTask *keyIdTask(const QString &key_filename);

/**
 * \brief Create a task for: `sleep SECONDS`
 */
CmdlineTask *sleepSecondsTask(int seconds);

#endif /* !TASKS_MISC_H */

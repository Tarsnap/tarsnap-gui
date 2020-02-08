#ifndef TASKS_SETUP_H
#define TASKS_SETUP_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

#include "tarsnaptask.h"

/**
 * \file tasks-setup.h
 * \brief These functions are used in the SetupWizard.
 *
 * They:
 * - should not have any side effects.
 * - should read information from TSettings (if possible), instead
 *   of using parameters.
 */

/**
 * \brief Create a task for: `tarsnap --version`
 */
TarsnapTask *tarsnapVersionTask();

/**
 * \brief Extract the version number from the output of `tarsnap --version`.
 *
 * \param stdOut standard output from `tarsnap --version`.
 */
const QString tarsnapVersionTaskParse(const QString &stdOut);

/**
 * \brief Create a task for: `tarsnap-keygen`
 */
TarsnapTask *registerMachineTask(const QString &password);

/**
 * \brief Create a task for: `tarsnap --fsck` or `tarsnap --fsck-prune`.
 */
TarsnapTask *fsckTask(bool prune);

/**
 * \brief Create a task for: `tarsnap --initialize-cachedir`.
 */
TarsnapTask *initializeCachedirTask();

#endif /* !TASKS_SETUP_H */

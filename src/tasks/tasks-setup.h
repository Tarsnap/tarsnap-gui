#ifndef TASKS_SETUP_H
#define TASKS_SETUP_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
class CmdlineTask;

/**
 * \file tasks-setup.h
 * \brief These functions are used in the SetupWizard.
 *
 * They:
 * - should not have any side effects.
 * - should read information from TSettings (if possible), instead
 *   of using parameters.
 * - functions returning CmdlineTask should be in the form twoWordsTask(),
 *   to distinguish them from oneTask variable names in TaskManager.
 */

/**
 * \brief Create a task for: `tarsnap --version`
 */
CmdlineTask *tarsnapVersionTask();

/**
 * \brief Extract the version number from the output of `tarsnap --version`.
 *
 * \param stdOut standard output from `tarsnap --version`.
 */
const QString tarsnapVersionTaskParse(const QString &stdOut);

/**
 * \brief Create a task for: `tarsnap-keygen`
 */
CmdlineTask *registerMachineTask(const QString &password,
                                 const QString &machine);

/**
 * \brief Create a task for: `tarsnap --fsck` or `tarsnap --fsck-prune`.
 */
CmdlineTask *fsckCleanTask(bool prune);

/**
 * \brief Create a task for: `tarsnap --initialize-cachedir`.
 */
CmdlineTask *initializeCachedirTask();

#endif /* !TASKS_SETUP_H */

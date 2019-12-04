#ifndef TASKS_UTILS_H
#define TASKS_UTILS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

/**
 * \file tasks-utils.h
 * \brief These functions support the `task-*.cpp` files.
 *
 * They:
 * - should not have any side effects.
 * - should read information from TSettings (if possible), instead
 *   of using parameters.
 */

/**
 * \brief Prepend the given \c cmd with the value of "`tarsnap/path`" in
 * the TSettings.
 *
 * \param cmd name of the binary (without directory).
 */
QString makeTarsnapCommand(QString cmd = "tarsnap");

/**
 * \brief Generate the usual arguments for a `tarsnap` command as specified
 * in the TSettings.
 *
 * In particular, these are `--keyfile`, `--cachedir`, `--maxbw-rate-down`,
 * `--maxbw-rate-up`, and `--no-default-config`.
 */
QStringList makeTarsnapArgs();

#endif /* !TASKS_UTILS_H */

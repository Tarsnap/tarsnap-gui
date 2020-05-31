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
QString makeTarsnapCommand(const QString &cmd = "tarsnap");

/**
 * \brief Generate the usual arguments for a `tarsnap` command as specified
 * in the TSettings.
 *
 * In particular, these are `--keyfile`, `--cachedir`, `--maxbw-rate-down`,
 * `--maxbw-rate-up`, and `--no-default-config`.
 */
QStringList makeTarsnapArgs();

/**
 * \brief Compare two version strings.
 *
 * \param found The detected version string.  Must consist of at least 3
 *              dot-separate values.  If the final element contains "-",
 *              it is split into another dot-separated value.
 * \param fixed A version string in the form "X.Y.Z"; will likely be a
 *              compile-time constant.
 */
int versionCompare(const QString &found, const QString &fixed);

#endif /* !TASKS_UTILS_H */

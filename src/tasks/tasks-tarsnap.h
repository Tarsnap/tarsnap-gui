#ifndef TASKS_TARSNAP_H
#define TASKS_TARSNAP_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

#include "persistentmodel/archive.h"

#include "backuptask.h"
#include "tarsnaptask.h"

/**
 * \file tasks-tarsnap.h
 * \brief These functions are used for Tarsnap-related tasks which
 * are not part of \ref tasks-setup.h .
 *
 * They:
 * - should not have any side effects.
 * - should read information from TSettings (if possible), instead
 *   of using parameters.
 */

/**
 * \brief Create a task for: `tarsnap --list-archives -vv`
 */
TarsnapTask *listArchivesTask();

/**
 * \brief Create a task for: `tarsnap --print-stats -f ARCHIVENAME`
 */
TarsnapTask *printStatsTask(const QString &archiveName);

/**
 * \brief Create a task for: `tarsnap -tv -f ARCHIVENAME`
 */
TarsnapTask *archiveContentsTask(const QString &archiveName);

/**
 * \brief Create a task for: `tarsnap -d -f ARCHIVENAME`
 */
TarsnapTask *deleteArchivesTask(const QStringList &archiveNames);

/**
 * \brief Create a task for: `tarsnap --print-stats`
 */
TarsnapTask *overallStatsTask();

/**
 * \brief Create a task for: `tarsnap --nuke`
 */
TarsnapTask *nukeArchivesTask();

/**
 * \brief Create a task for: `tarsnap -x -f ARCHIVENAME`, with many options.
 */
TarsnapTask *restoreArchiveTask(const QString &       archiveName,
                                ArchiveRestoreOptions options);

/**
 * \brief Create a task for: `tarsnap -c -f ARCHIVENAME FILELIST`, with
 * many options.
 */
TarsnapTask *backupArchiveTask(BackupTaskDataPtr backupTaskData);

/**
 * \brief Create a task for: `tarsnap-keymgmt --print-key-id KEYFILENAME`
 */
TarsnapTask *keyIdTask(const QString &key_filename);

#endif /* !TASKS_TARSNAP_H */

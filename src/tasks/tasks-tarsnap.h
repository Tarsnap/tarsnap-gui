#ifndef TASKS_TARSNAP_H
#define TASKS_TARSNAP_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

#include "persistentmodel/archive.h"

#include "backuptask.h"
#include "cmdlinetask.h"

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

/** Parsed result of `--print-stats`. */
struct tarsnap_stats
{
    /** Total size */
    quint64 total;
    /** Compressed size */
    quint64 compressed;
    /** Total size (unique data) */
    quint64 unique_total;
    /** Compressed size (unique data) */
    quint64 unique_compressed;
    /** Was there any error parsing the stats? */
    bool parse_error;
};

/**
 * \brief Create a task for: `tarsnap --list-archives -vv`
 */
CmdlineTask *listArchivesTask();

/**
 * \brief Create a task for: `tarsnap --print-stats -f ARCHIVENAME`
 */
CmdlineTask *printStatsTask(const QString &archiveName);

/**
 * \brief Create a task for: `tarsnap -tv -f ARCHIVENAME`
 */
CmdlineTask *archiveContentsTask(const QString &archiveName);

/**
 * \brief Create a task for: `tarsnap -d -f ARCHIVENAME`
 */
CmdlineTask *deleteArchivesTask(const QStringList &archiveNames);

/**
 * \brief Create a task for: `tarsnap --print-stats`
 */
CmdlineTask *overallStatsTask();

/**
 * \brief Create a task for: `tarsnap --nuke`
 */
CmdlineTask *nukeArchivesTask();

/**
 * \brief Create a task for: `tarsnap -x -f ARCHIVENAME`, with many options.
 */
CmdlineTask *restoreArchiveTask(const QString &       archiveName,
                                ArchiveRestoreOptions options);

/**
 * \brief Create a task for: `tarsnap -c -f ARCHIVENAME FILELIST`, with
 * many options.
 */
CmdlineTask *backupArchiveTask(BackupTaskDataPtr backupTaskData);

#endif /* !TASKS_TARSNAP_H */

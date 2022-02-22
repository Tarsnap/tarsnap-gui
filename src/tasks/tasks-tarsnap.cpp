#include "tasks/tasks-tarsnap.h"

WARNINGS_DISABLE
#include <QChar>
#include <QList>
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include "TSettings.h"

#include "messages/archiverestoreoptions.h"

#include "backuptask.h"
#include "cmdlinetask.h"
#include "compat.h"
#include "tasks/tasks-defs.h"
#include "tasks/tasks-utils.h"

CmdlineTask *listArchivesTask()
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    args << "--list-archives"
         << "-vv";

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

QList<struct archive_list_data>
listArchivesTaskParse(const QString &tarsnapOutput)
{
    QList<struct archive_list_data> metadatas;
    QStringList lines = tarsnapOutput.split('\n', SKIP_EMPTY_PARTS);
    for(const QString &line : lines)
    {
        QRegExp archiveDetailsRX("^(.+)\\t+(\\S+\\s+\\S+)\\t+(.+)$");
        if(-1 != archiveDetailsRX.indexIn(line))
        {
            struct archive_list_data metadata;
            QStringList archiveDetails = archiveDetailsRX.capturedTexts();
            archiveDetails.removeFirst();
            metadata.archiveName = archiveDetails[0];
            metadata.timestamp =
                QDateTime::fromString(archiveDetails[1], Qt::ISODate);
            metadata.command = archiveDetails[2];

            metadata.parse_error = false;
            metadatas.append(metadata);
        }
    }

    return metadatas;
}

CmdlineTask *printStatsTask(const QString &archiveName)
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    args << "--print-stats"
         << "--no-humanize-numbers"
         << "-f" << archiveName;

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

struct tarsnap_stats printStatsTaskParse(const QString &tarsnapOutput,
                                         bool           newArchiveOutput,
                                         const QString &archiveName)
{
    struct tarsnap_stats stats;
    stats.parse_error = true;

    QStringList lines = tarsnapOutput.split('\n', SKIP_EMPTY_PARTS);
    if(lines.count() < 5)
        return stats;

    QRegExp sizeRX;
    QRegExp uniqueSizeRX;
    if(newArchiveOutput)
    {
        sizeRX.setPattern("^This archive\\s+(\\d+)\\s+(\\d+)$");
        uniqueSizeRX.setPattern("^New data\\s+(\\d+)\\s+(\\d+)$");
    }
    else
    {
        sizeRX.setPattern(QString("^%1\\s+(\\d+)\\s+(\\d+)$").arg(archiveName));
        uniqueSizeRX.setPattern("^\\s+\\(unique data\\)\\s+(\\d+)\\s+(\\d+)$");
    }
    bool matched = false;
    for(const QString &line : lines)
    {
        if(-1 != sizeRX.indexIn(line))
        {
            QStringList captured = sizeRX.capturedTexts();
            captured.removeFirst();
            stats.total      = captured[0].toULongLong();
            stats.compressed = captured[1].toULongLong();

            matched = true;
        }
        if(-1 != uniqueSizeRX.indexIn(line))
        {
            QStringList captured = uniqueSizeRX.capturedTexts();
            captured.removeFirst();
            stats.unique_total      = captured[0].toULongLong();
            stats.unique_compressed = captured[1].toULongLong();
            matched                 = true;
        }
    }
    if(!matched)
        return stats;

    // We're ok.
    stats.parse_error = false;
    return stats;
}

CmdlineTask *archiveContentsTask(const QString &archiveName)
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    TSettings settings;
    if(settings.value("tarsnap/preserve_pathnames", DEFAULT_PRESERVE_PATHNAMES)
           .toBool())
        args << "-P";
    args << "-tv"
         << "-f" << archiveName;

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

CmdlineTask *deleteArchivesTask(const QStringList &archiveNames)
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    args << "--print-stats"
         << "-d";
    for(const QString &archiveName : archiveNames)
        args << "-f" << archiveName;

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

CmdlineTask *overallStatsTask()
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    args << "--print-stats"
         << "--no-humanize-numbers";

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

struct tarsnap_stats overallStatsTaskParse(const QString &tarsnapOutput)
{
    struct tarsnap_stats stats;
    stats.parse_error = true;

    QStringList lines = tarsnapOutput.split('\n', SKIP_EMPTY_PARTS);
    if(lines.count() < 3)
        return stats;

    QRegExp sizeRX("^All archives\\s+(\\d+)\\s+(\\d+)$");
    if(-1 == sizeRX.indexIn(lines[1]))
        return stats;

    QStringList captured = sizeRX.capturedTexts();
    captured.removeFirst();
    stats.total      = captured[0].toULongLong();
    stats.compressed = captured[1].toULongLong();

    QRegExp uniqueSizeRX("^\\s+\\(unique data\\)\\s+(\\d+)\\s+(\\d+)$");
    if(-1 == uniqueSizeRX.indexIn(lines[2]))
        return stats;

    captured = uniqueSizeRX.capturedTexts();
    captured.removeFirst();
    stats.unique_total      = captured[0].toULongLong();
    stats.unique_compressed = captured[1].toULongLong();

    // We're ok.
    stats.parse_error = false;
    return stats;
}

CmdlineTask *nukeArchivesTask()
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    args << "--nuke";
    task->setStdIn("No Tomorrow\n");

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

CmdlineTask *restoreArchiveTask(const QString        &archiveName,
                                ArchiveRestoreOptions options)
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    if(options.optionRestore)
    {
        TSettings settings;
        args << "-x"
             << "-P"
             << "-C"
             << settings.value("app/downloads_dir", DEFAULT_DOWNLOADS)
                    .toString();
    }
    if(options.optionRestoreDir)
    {
        args << "-x"
             << "-C" << options.path;
    }
    if((options.optionRestore || options.optionRestoreDir))
    {
        if(!options.overwriteFiles)
            args << "-k";
        if(options.keepNewerFiles)
            args << "--keep-newer-files";
        if(options.preservePerms)
            args << "-p";
    }
    if(options.optionTarArchive)
    {
        args << "-r";
        task->setStdOutFile(options.path);
    }
    if(!options.files.isEmpty())
    {
        args << "-T"
             << "-";
        task->setStdIn(options.files.join(QChar('\n')));
    }
    args << "-f" << archiveName;

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

CmdlineTask *backupArchiveTask(BackupTaskDataPtr backupTaskData)
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    TSettings settings;
    if(settings
           .value("tarsnap/aggressive_networking",
                  DEFAULT_AGGRESSIVE_NETWORKING)
           .toBool())
        args << "--aggressive-networking";
    if(backupTaskData->optionDryRun())
        args << "--dry-run";
    if(backupTaskData->optionSkipNoDump())
        args << "--nodump";
    if(backupTaskData->optionPreservePaths())
        args << "-P";
    if(!backupTaskData->optionTraverseMount())
        args << "--one-file-system";
    if(backupTaskData->optionFollowSymLinks())
        args << "-L";
    args << "--creationtime"
         << QString::number(backupTaskData->timestamp().toTime_t());
    args << "--quiet"
         << "--print-stats"
         << "--no-humanize-numbers"
         << "-c"
         << "-f" << backupTaskData->name();
    for(const QString &exclude : backupTaskData->getExcludesList())
        args << "--exclude" << exclude;
    for(const QUrl &url : backupTaskData->urls())
        args << url.toLocalFile();

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

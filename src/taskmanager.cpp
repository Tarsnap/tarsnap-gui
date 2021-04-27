#include "taskmanager.h"

WARNINGS_DISABLE
#include <QChar>
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QLatin1String>
#include <QMetaType>
#include <QStringList>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include "TSettings.h"

#include "messages/archiverestoreoptions.h"

#include "backenddata.h"
#include "backuptask.h"
#include "basetask.h"
#include "cmdlinetask.h"
#include "compat.h"
#include "debug.h"
#include "humanbytes.h"
#include "jobrunner.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "taskqueuer.h"
#include "tasks/tasks-defs.h"
#include "tasks/tasks-misc.h"
#include "tasks/tasks-setup.h"
#include "tasks/tasks-tarsnap.h"
#include "tasks/tasks-utils.h"

#define SUCCESS 0

Q_DECLARE_METATYPE(CmdlineTask *)

TaskManager::TaskManager() : QObject(), _bd(new BackendData())
{
    setupTaskQueuer();
}

TaskManager::~TaskManager()
{
    delete _bd;
    delete _tq;
    // Wait up to 1 second to delete objects scheduled with ->deleteLater()
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void TaskManager::setupTaskQueuer()
{
    _tq = new TaskQueuer();
    connect(_tq, &TaskQueuer::numTasks, this, &TaskManager::numTasks);
    connect(_tq, &TaskQueuer::message, this, &TaskManager::message);
}

void TaskManager::tarsnapVersionFind()
{
    CmdlineTask *versionTask = tarsnapVersionTask();
    connect(versionTask, &CmdlineTask::finished, this,
            &TaskManager::getTarsnapVersionFinished);
    _tq->queueTask(versionTask);
}

void TaskManager::registerMachineDo(const QString &password,
                                    const QString &machine,
                                    const bool     useExistingKeyfile)
{
    CmdlineTask *registerTask;
    CmdlineTask *secondTask = nullptr;
    QVariant     data;

    // Get relevant settings
    TSettings settings;
    QString   keyFilename = settings.value("tarsnap/key", "").toString();
    QString   cachePath   = settings.value("tarsnap/cache", "").toString();

    bool keyExists = QFileInfo::exists(keyFilename);

    // Sanity check: existing keyfile should exist; new keyfile shouldn't exist.
    if(useExistingKeyfile && !keyExists)
    {
        emit registerMachineDone(TaskStatus::Failed, "Keyfile does not exist");
        return;
    }
    if(!useExistingKeyfile && keyExists)
    {
        emit registerMachineDone(TaskStatus::Failed, "Keyfile already exists");
        return;
    }

    // Actual task(s)
    if(useExistingKeyfile)
    {
        // existing key, attempt to rebuild cache & verify archive integrity
        registerTask = fsckCleanTask(true);
        registerTask->setMonitorOutput();
        connect(registerTask, &CmdlineTask::outputStdout, this,
                &TaskManager::registerMachineProgress);
    }
    else
    {
        // generate a new key and register machine with tarsnap-keygen
        registerTask = registerMachineTask(password, machine);

        // second task: --initialize-cachedir or --fsck existing cachedir
        QDir cacheDir(cachePath);

        if(!cachePath.isEmpty() &&
#if(QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
           cacheDir.isEmpty()
#else
           !cacheDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)
                .count()
#endif
        )
            secondTask = initializeCachedirTask();
        else
            secondTask = fsckCleanTask(true);

        // We don't have a third task.
        // Compatibility: normally we'd just do data.setValue(nullptr), but
        // that doesn't compile on Qt 5.5.1 (although it's fine on Qt 5.13.0).
        data.setValue(static_cast<CmdlineTask *>(nullptr));
        secondTask->setData(data);
    }

    // Always set data; if secondTask is nullptr, we still need to know that
    // when we get to registerMachineFinished().
    data.setValue(secondTask);
    registerTask->setData(data);

    connect(registerTask, &CmdlineTask::finished, this,
            &TaskManager::registerMachineFinished);
    _tq->queueTask(registerTask);
}

void TaskManager::backupNow(BackupTaskDataPtr backupTaskData)
{
    if(backupTaskData == nullptr)
    {
        DEBUG << "Null BackupTaskDataPtr passed.";
        return;
    }

    CmdlineTask *backupTask = backupArchiveTask(backupTaskData);
    backupTaskData->setCommand(backupTask->command() + " "
                               + backupTask->arguments().join(" "));
    backupTask->setData(QVariant::fromValue(backupTaskData));
    connect(backupTask, &CmdlineTask::finished, this,
            &TaskManager::backupTaskFinished);
    connect(backupTask, &CmdlineTask::started, this,
            &TaskManager::backupTaskStarted);
    notifyBackupTaskUpdate(backupTaskData, TaskStatus::Queued);
    _tq->queueTask(backupTask, true, true);
}

void TaskManager::getArchives()
{
    CmdlineTask *listTask = listArchivesTask();
    listTask->setTruncateLogOutput(true);
    connect(listTask, &CmdlineTask::finished, this,
            &TaskManager::getArchiveListFinished);
    connect(listTask, &CmdlineTask::started, this, [this]() {
        emit message(tr("Updating archives list from remote..."));
    });
    _tq->queueTask(listTask);
}

void TaskManager::loadArchives()
{
    if(!_bd->loadArchives())
        return;
    QList<ArchivePtr> archives = _bd->archives().values();
    // Send update.
    emit archiveList(archives);
}

void TaskManager::getArchiveStats(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    CmdlineTask *statsTask = printStatsTask(archive->name());
    statsTask->setData(QVariant::fromValue(archive));
    connect(statsTask, &CmdlineTask::finished, this,
            &TaskManager::getArchiveStatsFinished);
    connect(statsTask, &CmdlineTask::started, this, [this, archive]() {
        emit message(
            tr("Fetching stats for archive <i>%1</i>...").arg(archive->name()));
    });
    _tq->queueTask(statsTask);
}

void TaskManager::getArchiveContents(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    CmdlineTask *contentsTask = archiveContentsTask(archive->name());
    contentsTask->setData(QVariant::fromValue(archive));
    contentsTask->setTruncateLogOutput(true);
    connect(contentsTask, &CmdlineTask::finished, this,
            &TaskManager::getArchiveContentsFinished);
    connect(contentsTask, &CmdlineTask::started, this, [this, archive]() {
        emit message(tr("Fetching contents for archive <i>%1</i>...")
                         .arg(archive->name()));
    });
    _tq->queueTask(contentsTask);
}

void TaskManager::deleteArchives(QList<ArchivePtr> archives)
{
    if(archives.isEmpty())
    {
        DEBUG << "Empty QList<ArchivePtr> passed.";
        return;
    }

    for(const ArchivePtr &archive : archives)
        archive->setDeleteScheduled(true);
    QStringList archiveNames;
    for(const ArchivePtr &archive : archives)
        archiveNames << archive->name();

    CmdlineTask *deleteTask = deleteArchivesTask(archiveNames);
    deleteTask->setData(QVariant::fromValue(archives));
    connect(deleteTask, &CmdlineTask::finished, this,
            &TaskManager::deleteArchivesFinished);
    connect(deleteTask, &CmdlineTask::canceled, this, [archives]() {
        for(const ArchivePtr &archive : archives)
            archive->setDeleteScheduled(false);
    });
    connect(deleteTask, &CmdlineTask::started, this,
            [this, archives]() { notifyArchivesDeleted(archives, false); });
    _tq->queueTask(deleteTask, true);
}

void TaskManager::getOverallStats()
{
    CmdlineTask *statsTask = overallStatsTask();
    connect(statsTask, &CmdlineTask::finished, this,
            &TaskManager::overallStatsFinished);
    _tq->queueTask(statsTask);
}

void TaskManager::fsck(bool prune)
{
    CmdlineTask *fsckTask = fsckCleanTask(prune);
    connect(fsckTask, &CmdlineTask::finished, this, &TaskManager::fsckFinished);
    connect(fsckTask, &CmdlineTask::started, this,
            [this]() { emit message(tr("Cache repair initiated.")); });
    _tq->queueTask(fsckTask, true);
}

void TaskManager::nuke()
{
    CmdlineTask *nukeTask = nukeArchivesTask();
    connect(nukeTask, &CmdlineTask::finished, this, &TaskManager::nukeFinished);
    connect(nukeTask, &CmdlineTask::started, this,
            [this]() { emit message(tr("Archives nuke initiated...")); });
    _tq->queueTask(nukeTask, true);
}

void TaskManager::restoreArchive(ArchivePtr                   archive,
                                 const ArchiveRestoreOptions &options)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    CmdlineTask *restoreTask = restoreArchiveTask(archive->name(), options);
    restoreTask->setData(QVariant::fromValue(archive));
    connect(restoreTask, &CmdlineTask::finished, this,
            &TaskManager::restoreArchiveFinished);
    connect(restoreTask, &CmdlineTask::started, this, [this, archive]() {
        emit message(
            tr("Restoring from archive <i>%1</i>...").arg(archive->name()));
    });
    _tq->queueTask(restoreTask);
}

void TaskManager::getKeyId(const QString &key_filename)
{
    QFileInfo keyFile(key_filename);
    if(!keyFile.exists())
    {
        DEBUG << "Invalid key path.";
        return;
    }
    CmdlineTask *keymgmtTask = keyIdTask(key_filename);
    keymgmtTask->setData(key_filename);
    connect(keymgmtTask, &CmdlineTask::finished, this,
            &TaskManager::getKeyIdFinished);
    _tq->queueTask(keymgmtTask);
}

void TaskManager::findMatchingArchives(const QString &jobPrefix)
{
    QList<ArchivePtr> matching = _bd->findMatchingArchives(jobPrefix);
    // Send response.
    emit matchingArchives(matching);
}

void TaskManager::runScheduledJobs()
{
    loadJobs();
    JobRunner jr;
    connect(&jr, &JobRunner::message, this, &TaskManager::message);
    connect(&jr, &JobRunner::displayNotification, this,
            &TaskManager::displayNotification);
    connect(&jr, &JobRunner::backup, this, &TaskManager::backupNow);
    jr.runScheduledJobs(_bd->jobs());
}

void TaskManager::stopTasks(bool interrupt, bool running, bool queued)
{
    _tq->stopTasks(interrupt, running, queued);
}

void TaskManager::backupTaskFinished(QVariant data, int exitCode,
                                     const QString &stdOut,
                                     const QString &stdErr)
{
    BackupTaskDataPtr backupTaskData = qvariant_cast<BackupTaskDataPtr>(data);
    if(!backupTaskData)
    {
        DEBUG << "Task not found: " << data.toUuid();
        return;
    }
    backupTaskData->setExitCode(exitCode);
    backupTaskData->setOutput(stdOut + stdErr);
    bool truncated = false;
    if(exitCode != SUCCESS)
    {
        int lastIndex =
            stdErr.lastIndexOf(QLatin1String("tarsnap: Archive truncated"), -1,
                               Qt::CaseSensitive);
        if(lastIndex == -1)
        {
            notifyBackupTaskUpdate(backupTaskData, TaskStatus::Failed);
            parseError(stdErr);
            return;
        }
        else
        {
            truncated = true;
        }
    }

    ArchivePtr archive = _bd->newArchive(backupTaskData, truncated);
    parseArchiveStats(stdErr, true, archive);
    // This needs the archive stats.
    notifyBackupTaskUpdate(backupTaskData, TaskStatus::Completed);

    // Bail if it's a --dry-run.
    if(backupTaskData->optionDryRun())
        return;

    // Write the Archive data to the PersistentStore.
    archive->save();

    emit archiveAdded(archive);

    parseGlobalStats(stdErr);
}

void TaskManager::backupTaskStarted(QVariant data)
{
    BackupTaskDataPtr backupTaskData = qvariant_cast<BackupTaskDataPtr>(data);
    notifyBackupTaskUpdate(backupTaskData, TaskStatus::Running);
}

void TaskManager::registerMachineFinished(QVariant data, int exitCode,
                                          const QString &stdOut,
                                          const QString &stdErr)
{
    // Retrieved the stored ("second") task.
    CmdlineTask *nextTask = data.value<CmdlineTask *>();

    // Handle error (if applicable; a "fake" task is not an error).
    if((exitCode != SUCCESS) && (exitCode != EXIT_FAKE_REQUEST))
    {
        QString err;

        if(stdErr.isEmpty())
        {
            if(exitCode == EXIT_DID_NOT_START)
                err = "Could not launch the command-line program";
            if(exitCode == EXIT_CMD_NOT_FOUND)
                err = "Could not find the command-line program";
            else if(exitCode == EXIT_CRASHED)
                err = "Crash occurred in the command-line program";
        }
        else
            err = stdErr;

        // Clean up second task (if applicable).
        if(nextTask != nullptr)
            delete nextTask;

        // We're done.
        emit registerMachineDone(TaskStatus::Failed, err);
        return;
    }

    if(nextTask != nullptr)
    {
        // Run the stored task.
        connect(nextTask, &CmdlineTask::finished, this,
                &TaskManager::registerMachineFinished);
        _tq->queueTask(nextTask);
        // We're not finished yet, so we want to let the event loop continue.
        return;
    }

    // We finished successfully
    emit registerMachineDone(TaskStatus::Completed, stdOut);
}

void TaskManager::getArchiveListFinished(QVariant data, int exitCode,
                                         const QString &stdOut,
                                         const QString &stdErr)
{
    Q_UNUSED(data)

    if(exitCode == SUCCESS)
    {
        emit message(tr("Updating archives list from remote... done."));
    }
    else
    {
        emit message(tr("Error: Failed to list archives from remote."),
                     tr("Tarsnap exited with code %1 and output:\n%2")
                         .arg(exitCode)
                         .arg(stdErr));
        parseError(stdErr);
        return;
    }

    QList<struct archive_list_data> metadatas = listArchivesTaskParse(stdOut);

    // Create & fill next archive list
    QList<ArchivePtr> newArchives = _bd->setArchivesFromList(metadatas);

    // Notify about new archives.
    for(const ArchivePtr &archive : newArchives)
        emit archiveAdded(archive);

    // Update stats.
    for(const ArchivePtr &archive : newArchives)
        getArchiveStats(archive);
    getOverallStats();
}

void TaskManager::getArchiveStatsFinished(QVariant data, int exitCode,
                                          const QString &stdOut,
                                          const QString &stdErr)
{
    ArchivePtr archive = data.value<ArchivePtr>();
    if(!archive)
    {
        DEBUG << "Archive not found.";
        return;
    }
    if(exitCode == SUCCESS)
    {
        emit message(tr("Fetching stats for archive <i>%1</i>... done.")
                         .arg(archive->name()));
    }
    else
    {
        emit message(tr("Error: Failed to get archive stats from remote."),
                     tr("Tarsnap exited with code %1 and output:\n%2")
                         .arg(exitCode)
                         .arg(stdErr));
        parseError(stdErr);
        return;
    }

    parseArchiveStats(stdOut, false, archive);
    // Write the Archive data to the PersistentStore.
    archive->save();

    parseGlobalStats(stdOut);
}

void TaskManager::getArchiveContentsFinished(QVariant data, int exitCode,
                                             const QString &stdOut,
                                             const QString &stdErr)
{
    ArchivePtr archive = data.value<ArchivePtr>();

    if(!archive)
    {
        DEBUG << "Archive not found.";
        return;
    }

    QString detailText;
    if(exitCode != SUCCESS)
    {
        bool truncated = stdErr.contains(QLatin1String("tarsnap: Truncated"
                                                       " input file"));
        if(archive->name().endsWith(".part", Qt::CaseSensitive) && truncated)
        {
            detailText = stdErr;
            archive->setTruncated(true);
            archive->setTruncatedInfo(stdErr);
        }
        else if(stdOut.isEmpty())
        {
            emit message(
                tr("Error: Failed to get archive contents from remote."),
                tr("Tarsnap exited with code %1 and output:\n%2")
                    .arg(exitCode)
                    .arg(stdErr));
            parseError(stdErr);
            return;
        }
    }

    emit message(tr("Fetching contents for archive <i>%1</i>... done.")
                     .arg(archive->name()),
                 detailText);

    archive->setContents(stdOut);
    archive->save();
}

void TaskManager::deleteArchivesFinished(QVariant data, int exitCode,
                                         const QString &stdOut,
                                         const QString &stdErr)
{
    Q_UNUSED(stdOut)
    QList<ArchivePtr> archives = data.value<QList<ArchivePtr>>();

    if(exitCode != SUCCESS)
    {
        emit message(tr("Error: Failed to delete archive(s) from remote."),
                     tr("Tarsnap exited with code %1 and output:\n%2")
                         .arg(exitCode)
                         .arg(stdErr));
        parseError(stdErr);
        for(const ArchivePtr &archive : archives)
            archive->setDeleteScheduled(false);
        return;
    }

    if(!archives.empty())
    {
        _bd->removeArchives(archives);
        notifyArchivesDeleted(archives, true);
    }
    // We are only interested in the output of the last archive deleted for
    // parsing the final global stats
    QStringList lines = stdErr.split('\n', SKIP_EMPTY_PARTS);
    QStringList lastFive;
    int         count = lines.count();
    for(int i = 0; i < qMin(5, count); ++i)
        lastFive.prepend(lines.takeLast());
    parseGlobalStats(lastFive.join('\n'));
}

void TaskManager::overallStatsFinished(QVariant data, int exitCode,
                                       const QString &stdOut,
                                       const QString &stdErr)
{
    Q_UNUSED(data);

    if(exitCode != SUCCESS)
    {
        emit message(tr("Error: Failed to get stats from remote."),
                     tr("Tarsnap exited with code %1 and output:\n%2")
                         .arg(exitCode)
                         .arg(stdErr));
        parseError(stdErr);
        return;
    }

    parseGlobalStats(stdOut);
}

void TaskManager::fsckFinished(QVariant data, int exitCode,
                               const QString &stdOut, const QString &stdErr)
{
    Q_UNUSED(data)
    if(exitCode == SUCCESS)
    {
        emit message(tr("Cache repair succeeded."), stdOut);
    }
    else
    {
        emit message(tr("Cache repair failed. Hover mouse for details."),
                     stdErr);
        parseError(stdErr);
    }
    getArchives();
}

void TaskManager::nukeFinished(QVariant data, int exitCode,
                               const QString &stdOut, const QString &stdErr)
{
    Q_UNUSED(data)
    if(exitCode == SUCCESS)
    {
        emit message(tr("All archives nuked successfully."), stdOut);
        fsck(false);
    }
    else
    {
        emit message(tr("Archives nuke failed. Hover mouse for details."),
                     stdErr);
        parseError(stdErr);
        return;
    }
}

void TaskManager::restoreArchiveFinished(QVariant data, int exitCode,
                                         const QString &stdOut,
                                         const QString &stdErr)
{
    Q_UNUSED(stdOut)
    ArchivePtr archive = data.value<ArchivePtr>();
    if(!archive)
    {
        DEBUG << "Archive not found.";
        return;
    }
    if(exitCode == SUCCESS)
    {
        emit message(tr("Restoring from archive <i>%1</i>... done.")
                         .arg(archive->name()));
    }
    else
    {
        emit message(tr("Restoring from archive <i>%1</i> failed."
                        " Hover mouse for details.")
                         .arg(archive->name()),
                     stdErr);
        parseError(stdErr);
        return;
    }
}

void TaskManager::notifyBackupTaskUpdate(BackupTaskDataPtr backupTaskData,
                                         const TaskStatus &status)
{
    if(!backupTaskData)
    {
        DEBUG << "Backup task update for invalid task";
        return;
    }
    switch(status)
    {
    case TaskStatus::Initialized:
        DEBUG << "Backup task undefined";
        break;
    case TaskStatus::Completed:
    {
        QString msg;

        // Was it a --dry-run, or a real archive?
        if(backupTaskData->optionDryRun())
            msg = tr("Simulated backup <i>%1</i> completed. (%2 new data would"
                     " have been added to Tarsnap)");
        else
            msg = tr("Backup <i>%1</i> completed. (%2 new data on Tarsnap)");
        // Fill in template
        msg = msg.arg(backupTaskData->name())
                  .arg(humanBytes(
                      backupTaskData->archive()->sizeUniqueCompressed()));

        emit message(msg, backupTaskData->archive()->archiveStats());
        emit displayNotification(msg, NOTIFICATION_ARCHIVE_CREATED,
                                 backupTaskData->name());
        break;
    }
    case TaskStatus::Queued:
        emit message(
            tr("Backup <i>%1</i> queued.").arg(backupTaskData->name()));
        break;
    case TaskStatus::Running:
    {
        QString msg =
            tr("Backup <i>%1</i> is running.").arg(backupTaskData->name());

        emit message(msg);
        emit displayNotification(msg, NOTIFICATION_ARCHIVE_CREATING,
                                 backupTaskData->name());
        break;
    }
    case TaskStatus::Failed:
    {
        QString msg =
            tr("Backup <i>%1</i> failed: %2")
                .arg(backupTaskData->name())
                .arg(backupTaskData->output()
                         .section(QChar('\n'), 0, 0, QString::SectionSkipEmpty)
                         .simplified());
        emit message(msg, backupTaskData->output());
        emit displayNotification(msg, NOTIFICATION_ARCHIVE_FAILED,
                                 backupTaskData->name());
        break;
    }
    case TaskStatus::Paused:
        emit message(
            tr("Backup <i>%1</i> paused.").arg(backupTaskData->name()));
        break;
    case TaskStatus::VersionTooLow:
        // It should be impossible to get here.
        Q_ASSERT(false);
        break;
    }
}

void TaskManager::notifyArchivesDeleted(QList<ArchivePtr> archives, bool done)
{
    if(archives.count() > 1)
    {
        QString detail(archives[0]->name());
        for(int i = 1; i < archives.count(); ++i)
        {
            ArchivePtr archive = archives.at(i);
            detail.append(QString::fromLatin1(", ") + archive->name());
        }
        emit message(tr("Deleting archive <i>%1</i> and %2 more archives... %3")
                         .arg(archives.first()->name())
                         .arg(archives.count() - 1)
                         .arg(done ? tr("done.") : ""),
                     detail);
    }
    else if(archives.count() == 1)
    {
        emit message(tr("Deleting archive <i>%1</i>... %2")
                         .arg(archives.first()->name())
                         .arg(done ? tr("done.") : ""));
    }
}

void TaskManager::getKeyIdFinished(QVariant data, int exitCode,
                                   const QString &stdOut, const QString &stdErr)
{
    QString key_filename = data.toString();
    if(exitCode == SUCCESS)
    {
        bool ok = false;
        // qulonglong is the same as quint64.
        quint64 id = stdOut.toULongLong(&ok);
        if(ok)
            emit keyId(key_filename, id);
        else
            DEBUG << "Invalid output from tarsnap-keymgmt for key "
                  << key_filename;
    }
    else
    {
        DEBUG << "Failed to get the id for key " << key_filename;
        parseError(stdErr);
    }
}

#ifdef QT_TESTLIB_LIB
void TaskManager::queueTask(BaseTask *task, bool exclusive, bool isBackup)
{
    _tq->queueTask(task, exclusive, isBackup);
}
#endif

void TaskManager::queueGuiTask(BaseTask *task)
{
    _tq->queueTask(task, false, false);
}

void TaskManager::cancelGuiTask(BaseTask *task, const QUuid &uuid)
{
    _tq->cancelTask(task, uuid);
}

void TaskManager::parseError(const QString &tarsnapOutput)
{
    if(tarsnapOutput.contains("Error reading cache directory")
       || tarsnapOutput.contains("Sequence number mismatch: Run --fsck")
       || tarsnapOutput.contains(
           "Directory is not consistent with archive: Run --fsck"))
    {
        emit error(TarsnapError::CacheError);
    }
    else if(tarsnapOutput.contains("Error fscking archives"))
    {
        emit error(TarsnapError::FsckError);
    }
    else if(tarsnapOutput.contains("Cannot obtain server address")
            || tarsnapOutput.contains("Error looking up")
            || tarsnapOutput.contains("Too many network failures"))
    {
        emit error(TarsnapError::NetworkError);
    }
}

void TaskManager::parseGlobalStats(const QString &tarsnapOutput)
{
    struct tarsnap_stats stats = overallStatsTaskParse(tarsnapOutput);

    // Bail if there's any error.
    if(stats.parse_error)
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }

    emit overallStats(stats.total, stats.compressed, stats.unique_total,
                      stats.unique_compressed, _bd->numArchives());
}

void TaskManager::parseArchiveStats(const QString &tarsnapOutput,
                                    bool newArchiveOutput, ArchivePtr archive)
{
    struct tarsnap_stats stats =
        printStatsTaskParse(tarsnapOutput, newArchiveOutput, archive->name());

    // Bail if there's any error.
    if(stats.parse_error)
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }

    archive->setSizeTotal(stats.total);
    archive->setSizeCompressed(stats.compressed);
    archive->setSizeUniqueTotal(stats.unique_total);
    archive->setSizeUniqueCompressed(stats.unique_compressed);
}

void TaskManager::loadJobs()
{
    if(!_bd->loadJobs())
        return;
    emit jobList(_bd->jobs());
}

void TaskManager::deleteJob(JobPtr job, bool purgeArchives)
{
    if(job)
    {
        _bd->deleteJob(job);

        if(purgeArchives)
        {
            emit message(tr("Job <i>%1</i> deleted. Deleting %2 associated "
                            "archives next...")
                             .arg(job->name())
                             .arg(job->archives().count()));
            deleteArchives(job->archives());
        }
        else
        {
            emit message(tr("Job <i>%1</i> deleted.").arg(job->name()));
        }
    }
}

void TaskManager::addJob(JobPtr job)
{
    _bd->addJob(job);
    emit message(tr("Job <i>%1</i> added.").arg(job->name()));
}

void TaskManager::getTarsnapVersionFinished(QVariant data, int exitCode,
                                            const QString &stdOut,
                                            const QString &stdErr)
{
    Q_UNUSED(data)

    if(exitCode != SUCCESS)
    {
        emit message(tr("Error: Failed to get Tarsnap version."),
                     tr("Tarsnap exited with code %1 and output:\n%2")
                         .arg(exitCode)
                         .arg(stdErr));
        emit tarsnapVersionFound(TaskStatus::Failed, "");
        return;
    }

    QString version = tarsnapVersionTaskParse(stdOut);
    if(versionCompare(version, TARSNAP_MIN_VERSION) < 0)
        emit tarsnapVersionFound(TaskStatus::VersionTooLow, version);
    else
        emit tarsnapVersionFound(TaskStatus::Completed, version);
}

#ifdef QT_TESTLIB_LIB
void TaskManager::fakeNextTask()
{
    _tq->fakeNextTask();
}

void TaskManager::waitUntilIdle()
{
    _tq->waitUntilIdle();
}
#endif

#ifdef QT_TESTLIB_LIB
void TaskManager::sleepSeconds(int seconds, bool exclusive)
{
    CmdlineTask *sleepTask = sleepSecondsTask(seconds);
    connect(sleepTask, &CmdlineTask::started, this,
            [this]() { emit message("Started sleep task."); });
    connect(sleepTask, &CmdlineTask::finished, this,
            [this]() { emit message("Finished sleep task."); });
    _tq->queueTask(sleepTask, exclusive);
}
#endif

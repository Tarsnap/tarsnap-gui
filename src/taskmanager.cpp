#include "taskmanager.h"

WARNINGS_DISABLE
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QTcpSocket>
#include <QTimer>
#include <QVariant>
WARNINGS_ENABLE

#include "backenddata.h"
#include "debug.h"
#include "jobrunner.h"
#include "taskqueuer.h"
#include "tasks-defs.h"
#include "tasks-misc.h"
#include "tasks-setup.h"
#include "tasks-tarsnap.h"
#include "tasks-utils.h"
#include "utils.h"

#include <TSettings.h>

#define SUCCESS 0

Q_DECLARE_METATYPE(CmdlineTask *)

TaskManager::TaskManager() : QObject(), _bd(new BackendData())
{
    setupTaskQueuer();
#ifdef QT_TESTLIB_LIB
    _fakeNextTask = false;
#endif
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

    _backupTaskMap[backupTaskData->uuid()] = backupTaskData;

    CmdlineTask *backupTask = backupArchiveTask(backupTaskData);
    backupTaskData->setCommand(backupTask->command() + " "
                               + backupTask->arguments().join(" "));
    backupTask->setData(backupTaskData->uuid());
    connect(backupTask, &CmdlineTask::finished, this,
            &TaskManager::backupTaskFinished);
    connect(backupTask, &CmdlineTask::started, this,
            &TaskManager::backupTaskStarted);
    connect(backupTaskData.data(), &BackupTaskData::statusUpdate, this,
            &TaskManager::notifyBackupTaskUpdate);
    backupTaskData->setStatus(TaskStatus::Queued);
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
    _archiveMap.clear();
    PersistentStore &store = PersistentStore::instance();
    if(!store.initialized())
    {
        DEBUG << "PersistentStore was not initialized properly.";
        return;
    }
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select name from archives")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    if(store.runQuery(query) && query.next())
    {
        do
        {
            ArchivePtr archive(new Archive);
            archive->setName(
                query.value(query.record().indexOf("name")).toString());
            archive->load();
            _archiveMap[archive->name()] = archive;
        } while(query.next());
    }
    QList<ArchivePtr> archives = _archiveMap.values();
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
    connect(deleteTask, &CmdlineTask::canceled, this, [](QVariant data) {
        QList<ArchivePtr> d_archives = data.value<QList<ArchivePtr>>();
        for(const ArchivePtr &archive : d_archives)
            archive->setDeleteScheduled(false);
    });
    connect(deleteTask, &CmdlineTask::started, this, [this](QVariant data) {
        QList<ArchivePtr> d_archives = data.value<QList<ArchivePtr>>();
        notifyArchivesDeleted(d_archives, false);
    });
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

void TaskManager::restoreArchive(ArchivePtr            archive,
                                 ArchiveRestoreOptions options)
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
    QList<ArchivePtr> matching;
    for(const ArchivePtr &archive : _archiveMap)
    {
        if(archive->name().startsWith(jobPrefix + QChar('_'))
           && archive->jobRef().isEmpty())
            matching << archive;
    }
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
    jr.runScheduledJobs(_jobMap);
}

void TaskManager::stopTasks(bool interrupt, bool running, bool queued)
{
    _tq->stopTasks(interrupt, running, queued);
}

void TaskManager::backupTaskFinished(QVariant data, int exitCode,
                                     const QString &stdOut,
                                     const QString &stdErr)
{
    BackupTaskDataPtr backupTaskData = _backupTaskMap[data.toUuid()];
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
            backupTaskData->setStatus(TaskStatus::Failed);
            parseError(stdErr);
            return;
        }
        else
        {
            truncated = true;
        }
    }

    ArchivePtr archive(new Archive);
    archive->setName(backupTaskData->name());
    if(truncated)
    {
        archive->setName(archive->name().append(".part"));
        archive->setTruncated(true);
    }
    archive->setCommand(backupTaskData->command());
    // Lose milliseconds precision by converting to Unix timestamp and back.
    // So that a subsequent comparison in getArchiveListFinished won't fail.
    archive->setTimestamp(
        QDateTime::fromTime_t(backupTaskData->timestamp().toTime_t()));
    archive->setJobRef(backupTaskData->jobRef());
    archive->save();
    backupTaskData->setArchive(archive);
    backupTaskData->setStatus(TaskStatus::Completed);
    _archiveMap.insert(archive->name(), archive);
    for(const JobPtr &job : _jobMap)
    {
        if(job->objectKey() == archive->jobRef())
            emit job->loadArchives();
    }
    parseArchiveStats(stdErr, true, archive);
    emit archiveAdded(archive);
    parseGlobalStats(stdErr);
}

void TaskManager::backupTaskStarted(QVariant data)
{
    BackupTaskDataPtr backupTaskData = _backupTaskMap[data.toString()];
    backupTaskData->setStatus(TaskStatus::Running);
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
    QList<ArchivePtr> newArchives;

    QMap<QString, ArchivePtr> nextArchiveMap;
    for(const struct archive_list_data &metadata : metadatas)
    {
        ArchivePtr archive =
            _archiveMap.value(metadata.archiveName, ArchivePtr(new Archive));
        if(!archive->objectKey().isEmpty()
           && (archive->timestamp() != metadata.timestamp))
        {
            // There is a different archive with the same name on the remote
            archive->purge();
            archive.clear();
            archive = archive.create();
        }
        if(archive->objectKey().isEmpty())
        {
            // New archive
            archive->setName(metadata.archiveName);
            archive->setTimestamp(metadata.timestamp);
            archive->setCommand(metadata.command);
            // Automagically set Job ownership
            for(const JobPtr &job : _jobMap)
            {
                if(archive->name().startsWith(job->archivePrefix()))
                    archive->setJobRef(job->objectKey());
            }
            archive->save();
            newArchives.append(archive);
        }
        nextArchiveMap.insert(archive->name(), archive);
        _archiveMap.remove(archive->name());
    }
    // Purge archives left in old _archiveMap (not mirrored by the remote)
    for(const ArchivePtr &archive : _archiveMap)
    {
        archive->purge();
    }
    _archiveMap.clear();
    _archiveMap = nextArchiveMap;
    for(const JobPtr &job : _jobMap)
    {
        emit job->loadArchives();
    }

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
        for(const ArchivePtr &archive : archives)
        {
            _archiveMap.remove(archive->name());
            archive->purge();
        }
        notifyArchivesDeleted(archives, true);
    }
    // We are only interested in the output of the last archive deleted for
    // parsing the final global stats
    QStringList lines = stdErr.split('\n', QString::SkipEmptyParts);
    QStringList lastFive;
    int         count = lines.count();
    for(int i = 0; i < std::min(5, count); ++i)
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

void TaskManager::notifyBackupTaskUpdate(QUuid uuid, const TaskStatus &status)
{
    BackupTaskDataPtr backupTaskData = _backupTaskMap[uuid];
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
        QString msg =
            tr("Backup <i>%1</i> completed. (%2 new data on Tarsnap)")
                .arg(backupTaskData->name())
                .arg(Utils::humanBytes(
                    backupTaskData->archive()->sizeUniqueCompressed()));
        emit message(msg, backupTaskData->archive()->archiveStats());
        emit displayNotification(msg, NOTIFICATION_ARCHIVE_CREATED,
                                 backupTaskData->name());
        _backupTaskMap.remove(backupTaskData->uuid());
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
        _backupTaskMap.remove(backupTaskData->uuid());
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
void TaskManager::queueTask(CmdlineTask *task, bool exclusive, bool isBackup)
{
    _tq->queueTask(task, exclusive, isBackup);
}
#endif

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
                      stats.unique_compressed,
                      static_cast<quint64>(_archiveMap.count()));
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

    archive->save();
}

void TaskManager::loadJobs()
{
    _jobMap.clear();
    PersistentStore &store = PersistentStore::instance();
    if(!store.initialized())
    {
        DEBUG << "PersistentStore was not initialized properly.";
        return;
    }
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select name from jobs")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    if(store.runQuery(query) && query.next())
    {
        do
        {
            JobPtr job(new Job);
            job->setName(
                query.value(query.record().indexOf("name")).toString());
            connect(job.data(), &Job::loadArchives, this,
                    &TaskManager::loadJobArchives);
            job->load();
            _jobMap[job->name()] = job;
        } while(query.next());
    }
    emit jobList(_jobMap);
}

void TaskManager::deleteJob(JobPtr job, bool purgeArchives)
{
    if(job)
    {
        // Clear JobRef for assigned Archives.
        for(const ArchivePtr &archive : job->archives())
        {
            archive->setJobRef("");
            archive->save();
        }

        job->purge();
        _jobMap.remove(job->name());

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

void TaskManager::loadJobArchives()
{
    Job *             job = qobject_cast<Job *>(sender());
    QList<ArchivePtr> archives;
    for(const ArchivePtr &archive : _archiveMap)
    {
        if(archive->jobRef() == job->objectKey())
            archives << archive;
    }
    job->setArchives(archives);
}

void TaskManager::addJob(JobPtr job)
{
    _jobMap[job->name()] = job;
    connect(job.data(), &Job::loadArchives, this,
            &TaskManager::loadJobArchives);
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

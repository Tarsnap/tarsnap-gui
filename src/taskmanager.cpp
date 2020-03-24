#include "taskmanager.h"

WARNINGS_DISABLE
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QTcpSocket>
#include <QTimer>
#include <QVariant>
WARNINGS_ENABLE

#include "debug.h"
#include "tasks-defs.h"
#include "tasks-misc.h"
#include "tasks-setup.h"
#include "tasks-tarsnap.h"
#include "tasks-utils.h"
#include "utils.h"

#include <TSettings.h>

#define SUCCESS 0

Q_DECLARE_METATYPE(TarsnapTask *)

TaskManager::TaskManager() : _threadPool(QThreadPool::globalInstance())
{
#ifdef QT_TESTLIB_LIB
    _fakeNextTask = false;
#endif
}

TaskManager::~TaskManager()
{
    // Wait up to 1 second to finish any background tasks
    _threadPool->waitForDone(1000);
    // Wait up to 1 second to delete objects scheduled with ->deleteLater()
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void TaskManager::tarsnapVersionFind()
{
    TarsnapTask *versionTask = tarsnapVersionTask();
    connect(versionTask, &TarsnapTask::finished, this,
            &TaskManager::getTarsnapVersionFinished);
    queueTask(versionTask);
}

void TaskManager::registerMachineDo(const QString &password,
                                    const QString &machine,
                                    const bool     useExistingKeyfile)
{
    TarsnapTask *registerTask;
    TarsnapTask *secondTask = nullptr;
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
        data.setValue(static_cast<TarsnapTask *>(nullptr));
        secondTask->setData(data);
    }

    // Always set data; if secondTask is nullptr, we still need to know that
    // when we get to registerMachineFinished().
    data.setValue(secondTask);
    registerTask->setData(data);

    connect(registerTask, &TarsnapTask::finished, this,
            &TaskManager::registerMachineFinished);
    queueTask(registerTask);
}

void TaskManager::backupNow(BackupTaskDataPtr backupTaskData)
{
    if(backupTaskData == nullptr)
    {
        DEBUG << "Null BackupTaskDataPtr passed.";
        return;
    }

    _backupTaskMap[backupTaskData->uuid()] = backupTaskData;

    TarsnapTask *backupTask = backupArchiveTask(backupTaskData);
    backupTaskData->setCommand(backupTask->command() + " "
                               + backupTask->arguments().join(" "));
    backupTask->setData(backupTaskData->uuid());
    connect(backupTask, &TarsnapTask::finished, this,
            &TaskManager::backupTaskFinished);
    connect(backupTask, &TarsnapTask::started, this,
            &TaskManager::backupTaskStarted);
    connect(backupTaskData.data(), &BackupTaskData::statusUpdate, this,
            &TaskManager::notifyBackupTaskUpdate);
    backupTaskData->setStatus(TaskStatus::Queued);
    queueTask(backupTask, true);
}

void TaskManager::getArchives()
{
    TarsnapTask *listTask = listArchivesTask();
    listTask->setTruncateLogOutput(true);
    connect(listTask, &TarsnapTask::finished, this,
            &TaskManager::getArchiveListFinished);
    connect(listTask, &TarsnapTask::started, this, [this]() {
        emit message(tr("Updating archives list from remote..."));
    });
    queueTask(listTask);
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
    emit archiveList(_archiveMap.values());
}

void TaskManager::getArchiveStats(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    TarsnapTask *statsTask = printStatsTask(archive->name());
    statsTask->setData(QVariant::fromValue(archive));
    connect(statsTask, &TarsnapTask::finished, this,
            &TaskManager::getArchiveStatsFinished);
    connect(statsTask, &TarsnapTask::started, this, [this, archive]() {
        emit message(
            tr("Fetching stats for archive <i>%1</i>...").arg(archive->name()));
    });
    queueTask(statsTask);
}

void TaskManager::getArchiveContents(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    TarsnapTask *contentsTask = archiveContentsTask(archive->name());
    contentsTask->setData(QVariant::fromValue(archive));
    contentsTask->setTruncateLogOutput(true);
    connect(contentsTask, &TarsnapTask::finished, this,
            &TaskManager::getArchiveContentsFinished);
    connect(contentsTask, &TarsnapTask::started, this, [this, archive]() {
        emit message(tr("Fetching contents for archive <i>%1</i>...")
                         .arg(archive->name()));
    });
    queueTask(contentsTask);
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

    TarsnapTask *deleteTask = deleteArchivesTask(archiveNames);
    deleteTask->setData(QVariant::fromValue(archives));
    connect(deleteTask, &TarsnapTask::finished, this,
            &TaskManager::deleteArchivesFinished);
    connect(deleteTask, &TarsnapTask::canceled, this, [](QVariant data) {
        QList<ArchivePtr> d_archives = data.value<QList<ArchivePtr>>();
        for(const ArchivePtr &archive : d_archives)
            archive->setDeleteScheduled(false);
    });
    connect(deleteTask, &TarsnapTask::started, this, [this](QVariant data) {
        QList<ArchivePtr> d_archives = data.value<QList<ArchivePtr>>();
        notifyArchivesDeleted(d_archives, false);
    });
    queueTask(deleteTask, true);
}

void TaskManager::getOverallStats()
{
    TarsnapTask *statsTask = overallStatsTask();
    connect(statsTask, &TarsnapTask::finished, this,
            &TaskManager::overallStatsFinished);
    queueTask(statsTask);
}

void TaskManager::fsck(bool prune)
{
    TarsnapTask *fsckTask = fsckCleanTask(prune);
    connect(fsckTask, &TarsnapTask::finished, this, &TaskManager::fsckFinished);
    connect(fsckTask, &TarsnapTask::started, this,
            [this]() { emit message(tr("Cache repair initiated.")); });
    queueTask(fsckTask, true);
}

void TaskManager::nuke()
{
    TarsnapTask *nukeTask = nukeArchivesTask();
    connect(nukeTask, &TarsnapTask::finished, this, &TaskManager::nukeFinished);
    connect(nukeTask, &TarsnapTask::started, this,
            [this]() { emit message(tr("Archives nuke initiated...")); });
    queueTask(nukeTask, true);
}

void TaskManager::restoreArchive(ArchivePtr            archive,
                                 ArchiveRestoreOptions options)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    TarsnapTask *restoreTask = restoreArchiveTask(archive->name(), options);
    restoreTask->setData(QVariant::fromValue(archive));
    connect(restoreTask, &TarsnapTask::finished, this,
            &TaskManager::restoreArchiveFinished);
    connect(restoreTask, &TarsnapTask::started, this, [this, archive]() {
        emit message(
            tr("Restoring from archive <i>%1</i>...").arg(archive->name()));
    });
    queueTask(restoreTask);
}

void TaskManager::getKeyId(const QString &key_filename)
{
    QFileInfo keyFile(key_filename);
    if(!keyFile.exists())
    {
        DEBUG << "Invalid key path.";
        return;
    }
    TarsnapTask *keymgmtTask = keyIdTask(key_filename);
    keymgmtTask->setData(key_filename);
    connect(keymgmtTask, &TarsnapTask::finished, this,
            &TaskManager::getKeyIdFinished);
    queueTask(keymgmtTask);
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
    emit matchingArchives(matching);
}

bool TaskManager::waitForOnline()
{
    for(int i = 0; i < NETWORK_UP_ATTEMPTS; i++)
    {
        QTcpSocket sock;
        sock.connectToHost(NETWORK_HOST, NETWORK_PORT);
        // Multiple calls to waitForConnected() do not accumulate time, so we
        // use ::sleep() below.
        if(sock.waitForConnected(500))
        {
            sock.disconnectFromHost();
            return true;
        }
        // Wait a bit, then try again
        if(i < NETWORK_UP_ATTEMPTS - 1)
            QThread::sleep(NETWORK_UP_SLEEP);
    }
    warnNotOnline();
    return false;
}

void TaskManager::warnNotOnline()
{
    QString title(tr("Scheduled jobs not executed (cannot reach server)."));
    QString body(tr("Please check your internet connectivity and try again."));

    emit message(title, body);
    emit displayNotification(title + "\n" + body, NOTIFICATION_NOT_ONLINE, "");
    // Quit with a delay to allow for the system notifications to go through
    QTimer *quitTimer = new QTimer(this);
    quitTimer->setSingleShot(true);
    connect(quitTimer, &QTimer::timeout, qApp, QCoreApplication::quit);
    quitTimer->start(1000);
}

void TaskManager::runScheduledJobs()
{
    loadJobs();
    TSettings settings;
    QDate     now(QDate::currentDate());
    QDate nextDaily  = settings.value("app/next_daily_timestamp", "").toDate();
    QDate nextWeekly = settings.value("app/next_weekly_timestamp", "").toDate();
    QDate nextMonthly =
        settings.value("app/next_monthly_timestamp", "").toDate();
    bool doDaily   = false;
    bool doWeekly  = false;
    bool doMonthly = false;
    if(!nextDaily.isValid() || (nextDaily <= now))
    {
        doDaily = true;
        settings.setValue("app/next_daily_timestamp", now.addDays(1));
    }
    if(!nextWeekly.isValid() || (nextWeekly <= now))
    {
        doWeekly         = true;
        QDate nextSunday = now.addDays(1);
        for(; nextSunday.dayOfWeek() != 7; nextSunday = nextSunday.addDays(1))
            /* Do nothing. */
            ;
        settings.setValue("app/next_weekly_timestamp", nextSunday);
    }
    if(!nextMonthly.isValid() || (nextMonthly <= now))
    {
        doMonthly       = true;
        QDate nextMonth = now.addMonths(1);
        nextMonth.setDate(nextMonth.year(), nextMonth.month(), 1);
        settings.setValue("app/next_monthly_timestamp", nextMonth);
    }
    settings.sync();
    DEBUG << "Daily: " << doDaily;
    DEBUG << "Next daily: "
          << settings.value("app/next_daily_timestamp", "").toDate().toString();
    DEBUG << "Weekly: " << doWeekly;
    DEBUG
        << "Next weekly: "
        << settings.value("app/next_weekly_timestamp", "").toDate().toString();
    DEBUG << "Monthly: " << doWeekly;
    DEBUG
        << "Next monthly: "
        << settings.value("app/next_monthly_timestamp", "").toDate().toString();
    bool nothingToDo = true;
    for(const JobPtr &job : _jobMap)
    {
        // Do we need to run any jobs?
        if((doDaily && (job->optionScheduledEnabled() == JobSchedule::Daily))
           || (doWeekly
               && (job->optionScheduledEnabled() == JobSchedule::Weekly))
           || (doMonthly
               && (job->optionScheduledEnabled() == JobSchedule::Monthly)))
        {
            // Before the first job...
            if(nothingToDo)
            {
                // ... we have a job now
                nothingToDo = false;
                // ... check & wait for an internet connection
                if(!waitForOnline())
                    return;
            }
            backupNow(job->createBackupTask());
        }
    }
    if(nothingToDo)
        qApp->quit();
}

void TaskManager::stopTasks(bool interrupt, bool running, bool queued)
{
    if(queued) // queued should be cleared first to avoid race
    {
        while(!_taskQueue.isEmpty())
        {
            TarsnapTask *task = _taskQueue.dequeue();
            if(task)
            {
                task->emitCanceled();
                task->deleteLater();
            }
        }
        emit message("Cleared queued tasks.");
    }
    if(interrupt)
    {
        // Sending a SIGQUIT will cause the tarsnap binary to
        // create a checkpoint.  Non-tarsnap binaries should be
        // receive a TarsnapTask::stop() instead of a SIGQUIT.
        if(!_runningTasks.isEmpty())
            _runningTasks.first()->sigquit();
        emit message("Interrupting current backup.");
    }
    if(running)
    {
        for(TarsnapTask *task : _runningTasks)
        {
            if(task)
                task->stop();
        }
        emit message("Stopped running tasks.");
    }
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
    parseArchiveStats(stdErr, true, archive);
    archive->save();
    backupTaskData->setArchive(archive);
    backupTaskData->setStatus(TaskStatus::Completed);
    _archiveMap.insert(archive->name(), archive);
    for(const JobPtr &job : _jobMap)
    {
        if(job->objectKey() == archive->jobRef())
            emit job->loadArchives();
    }
    emit addArchive(archive);
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
    TarsnapTask *nextTask = data.value<TarsnapTask *>();

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
        connect(nextTask, &TarsnapTask::finished, this,
                &TaskManager::registerMachineFinished);
        queueTask(nextTask);
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

    QMap<QString, ArchivePtr> _newArchiveMap;
    QStringList lines = stdOut.split('\n', QString::SkipEmptyParts);
    for(const QString &line : lines)
    {
        QRegExp archiveDetailsRX("^(.+)\\t+(\\S+\\s+\\S+)\\t+(.+)$");
        if(-1 != archiveDetailsRX.indexIn(line))
        {
            QStringList archiveDetails = archiveDetailsRX.capturedTexts();
            archiveDetails.removeFirst();
            QDateTime timestamp =
                QDateTime::fromString(archiveDetails[1], Qt::ISODate);
            ArchivePtr archive =
                _archiveMap.value(archiveDetails[0], ArchivePtr(new Archive));
            if(!archive->objectKey().isEmpty()
               && (archive->timestamp() != timestamp))
            {
                // There is a different archive with the same name on the remote
                archive->purge();
                archive.clear();
                archive = archive.create();
            }
            if(archive->objectKey().isEmpty())
            {
                // New archive
                archive->setName(archiveDetails[0]);
                archive->setTimestamp(timestamp);
                archive->setCommand(archiveDetails[2]);
                // Automagically set Job ownership
                for(const JobPtr &job : _jobMap)
                {
                    if(archive->name().startsWith(job->archivePrefix()))
                        archive->setJobRef(job->objectKey());
                }
                archive->save();
                emit addArchive(archive);
                getArchiveStats(archive);
            }
            _newArchiveMap.insert(archive->name(), archive);
            _archiveMap.remove(archive->name());
        }
    }
    // Purge archives left in old _archiveMap (not mirrored by the remote)
    for(const ArchivePtr &archive : _archiveMap)
    {
        archive->purge();
    }
    _archiveMap.clear();
    _archiveMap = _newArchiveMap;
    for(const JobPtr &job : _jobMap)
    {
        emit job->loadArchives();
    }
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

void TaskManager::queueTask(TarsnapTask *task, bool exclusive)
{
    if(task == nullptr)
    {
        DEBUG << "NULL argument";
        return;
    }
    if(exclusive && !_runningTasks.isEmpty())
        _taskQueue.enqueue(task);
    else
        startTask(task);
}

void TaskManager::startTask(TarsnapTask *task)
{
    if(task == nullptr)
    {
        if(!_taskQueue.isEmpty())
            task = _taskQueue.dequeue();
        else
            return;
    }
    connect(task, &TarsnapTask::dequeue, this, &TaskManager::dequeueTask);

    // Record this thread as "running", even though it hasn't actually
    // started yet.  QThreadPool::start() is non-blocking, and in fact
    // explicitly states that a QRunnable can be added to an internal
    // run queue if it's exceeded QThreadPoll::maxThreadCount().
    //
    // However, for the purpose of this TaskManager, the task should not
    // be recorded in our _taskQueue (because we've just dequeued()'d it).
    // The "strictly correct" solution would be to add a
    // _waitingForStart queue, and move items out of that queue when the
    // relevant TarsnapTask::started signal was emitted.  At the moment,
    // I don't think that step is necessary, but I might need to revisit
    // that decision later.
    _runningTasks.append(task);

    task->setAutoDelete(false);
#ifdef QT_TESTLIB_LIB
    if(_fakeNextTask)
        task->fake();
#endif
    _threadPool->start(task);
    emit idle(false);
    bool backupTaskRunning = isBackupTaskRunning();
    emit numTasks(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
}

void TaskManager::dequeueTask()
{
    TarsnapTask *task = qobject_cast<TarsnapTask *>(sender());
    if(task == nullptr)
        return;
    _runningTasks.removeOne(task);
    task->deleteLater();
    if(_runningTasks.isEmpty())
    {
        if(_taskQueue.isEmpty())
            emit idle(true);
        else
            startTask(nullptr); // start another queued task
    }
    bool backupTaskRunning = isBackupTaskRunning();
    emit numTasks(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
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
    QStringList lines = tarsnapOutput.split('\n', QString::SkipEmptyParts);
    if(lines.count() < 3)
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }

    quint64 sizeTotal            = 0;
    quint64 sizeCompressed       = 0;
    quint64 sizeUniqueTotal      = 0;
    quint64 sizeUniqueCompressed = 0;

    QRegExp sizeRX("^All archives\\s+(\\d+)\\s+(\\d+)$");
    if(-1 == sizeRX.indexIn(lines[1]))
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }

    QStringList captured = sizeRX.capturedTexts();
    captured.removeFirst();
    sizeTotal      = captured[0].toULongLong();
    sizeCompressed = captured[1].toULongLong();

    QRegExp uniqueSizeRX("^\\s+\\(unique data\\)\\s+(\\d+)\\s+(\\d+)$");
    if(-1 == uniqueSizeRX.indexIn(lines[2]))
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }

    captured = uniqueSizeRX.capturedTexts();
    captured.removeFirst();
    sizeUniqueTotal      = captured[0].toULongLong();
    sizeUniqueCompressed = captured[1].toULongLong();

    emit overallStats(sizeTotal, sizeCompressed, sizeUniqueTotal,
                      sizeUniqueCompressed,
                      static_cast<quint64>(_archiveMap.count()));
}

void TaskManager::parseArchiveStats(const QString &tarsnapOutput,
                                    bool newArchiveOutput, ArchivePtr archive)
{
    QStringList lines = tarsnapOutput.split('\n', QString::SkipEmptyParts);
    if(lines.count() < 5)
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    QRegExp sizeRX;
    QRegExp uniqueSizeRX;
    if(newArchiveOutput)
    {
        sizeRX.setPattern("^This archive\\s+(\\d+)\\s+(\\d+)$");
        uniqueSizeRX.setPattern("^New data\\s+(\\d+)\\s+(\\d+)$");
    }
    else
    {
        sizeRX.setPattern(
            QString("^%1\\s+(\\d+)\\s+(\\d+)$").arg(archive->name()));
        uniqueSizeRX.setPattern("^\\s+\\(unique data\\)\\s+(\\d+)\\s+(\\d+)$");
    }
    bool matched = false;
    for(const QString &line : lines)
    {
        if(-1 != sizeRX.indexIn(line))
        {
            QStringList captured = sizeRX.capturedTexts();
            captured.removeFirst();
            archive->setSizeTotal(captured[0].toULongLong());
            archive->setSizeCompressed(captured[1].toULongLong());
            matched = true;
        }
        if(-1 != uniqueSizeRX.indexIn(line))
        {
            QStringList captured = uniqueSizeRX.capturedTexts();
            captured.removeFirst();
            archive->setSizeUniqueTotal(captured[0].toULongLong());
            archive->setSizeUniqueCompressed(captured[1].toULongLong());
            matched = true;
        }
    }
    if(!matched)
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    archive->save();
}

QString TaskManager::makeTarsnapCommand(const QString &cmd)
{
    TSettings settings;
    QString   _tarsnapDir = settings.value("tarsnap/path", "").toString();
    if(_tarsnapDir.isEmpty())
        return cmd;
    else
        return _tarsnapDir + QDir::separator() + cmd;
}

void TaskManager::initTarsnapArgs(QStringList &args)
{
    TSettings settings;
    QString   tarsnapKeyFile = settings.value("tarsnap/key", "").toString();
    if(!tarsnapKeyFile.isEmpty())
        args << "--keyfile" << tarsnapKeyFile;
    QString tarsnapCacheDir = settings.value("tarsnap/cache", "").toString();
    if(!tarsnapCacheDir.isEmpty())
        args << "--cachedir" << tarsnapCacheDir;
    int download_rate_kbps = settings.value("app/limit_download", 0).toInt();
    if(download_rate_kbps)
    {
        args.prepend("--maxbw-rate-down");
        args.insert(1, QString::number(1024 * quint64(download_rate_kbps)));
    }
    int upload_rate_kbps = settings.value("app/limit_upload", 0).toInt();
    if(upload_rate_kbps)
    {
        args.prepend("--maxbw-rate-up");
        args.insert(1, QString::number(1024 * quint64(upload_rate_kbps)));
    }
    if(settings.value("tarsnap/no_default_config", DEFAULT_NO_DEFAULT_CONFIG)
           .toBool())
        args.prepend("--no-default-config");
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

bool TaskManager::isBackupTaskRunning()
{
    if(!_runningTasks.isEmpty() && !_backupTaskMap.isEmpty())
    {
        for(TarsnapTask *task : _runningTasks)
        {
            if(task && _backupTaskMap.contains(task->data().toUuid()))
            {
                return true;
            }
        }
    }
    return false;
}

void TaskManager::getTaskInfo()
{
    bool backupTaskRunning = isBackupTaskRunning();
    emit taskInfo(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
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
    _fakeNextTask = true;
}

void TaskManager::waitUntilIdle()
{
    while(!(_taskQueue.isEmpty() && _runningTasks.isEmpty()))
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
#endif

#ifdef QT_TESTLIB_LIB
void TaskManager::sleepSeconds(int seconds, bool exclusive)
{
    TarsnapTask *sleepTask = sleepSecondsTask(seconds);
    connect(sleepTask, &TarsnapTask::started, this,
            [this]() { emit message("Started sleep task."); });
    connect(sleepTask, &TarsnapTask::finished, this,
            [this]() { emit message("Finished sleep task."); });
    queueTask(sleepTask, exclusive);
}
#endif

#include "taskmanager.h"

#include "debug.h"
#include "utils.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QTcpSocket>
#include <QTimer>

#define SUCCESS 0

#define NETWORK_HOST "v1-0-0-server.tarsnap.com"
#define NETWORK_PORT 9279
#define NETWORK_UP_SLEEP 60
#define NETWORK_UP_ATTEMPTS 3

TaskManager::TaskManager() : _threadPool(QThreadPool::globalInstance())
{
}

TaskManager::~TaskManager()
{
}

void TaskManager::getTarsnapVersion(QString tarsnapPath)
{
    TarsnapTask *tarsnap = new TarsnapTask();
    if(tarsnapPath.isEmpty())
        tarsnap->setCommand(CMD_TARSNAP);
    else
        tarsnap->setCommand(tarsnapPath + QDir::separator() + CMD_TARSNAP);
    tarsnap->setArguments(QStringList("--version"));
    connect(tarsnap, &TarsnapTask::finished, this,
            &TaskManager::getTarsnapVersionFinished, QUEUED);
    queueTask(tarsnap);
}

void TaskManager::registerMachine(QString user, QString password,
                                  QString machine, QString key,
                                  QString tarsnapPath, QString cachePath)
{
    TarsnapTask *registerTask = new TarsnapTask();
    QStringList  args;
    if(QFileInfo(key).exists())
    {
        // existing key, attempt to rebuild cache & verify archive integrity
        args << "--fsck-prune"
             << "--keyfile" << key << "--cachedir" << cachePath;
        registerTask->setCommand(tarsnapPath + QDir::separator() + CMD_TARSNAP);
        registerTask->setArguments(args);
    }
    else
    {
        // generate a new key and register machine with tarsnap-keygen
        args << "--user" << user << "--machine" << machine << "--keyfile" << key;
        registerTask->setCommand(tarsnapPath + QDir::separator()
                                 + CMD_TARSNAPKEYGEN);
        registerTask->setArguments(args);
        registerTask->setStdIn(password);
    }
    connect(registerTask, &TarsnapTask::finished, this,
            &TaskManager::registerMachineFinished, QUEUED);
    queueTask(registerTask);
}

void TaskManager::backupNow(BackupTaskPtr backupTask)
{
    if(backupTask == nullptr)
    {
        DEBUG << "Null BackupTaskPtr passed.";
        return;
    }

    _backupTaskMap[backupTask->uuid()] = backupTask;
    TarsnapTask *bTask                 = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    QSettings settings;
    if(settings
           .value("tarsnap/aggressive_networking", DEFAULT_AGGRESSIVE_NETWORKING)
           .toBool())
        args << "--aggressive-networking";
    if(backupTask->optionDryRun())
        args << "--dry-run";
    if(backupTask->optionSkipNoDump())
        args << "--nodump";
    if(backupTask->optionPreservePaths())
        args << "-P";
    if(!backupTask->optionTraverseMount())
        args << "--one-file-system";
    if(backupTask->optionFollowSymLinks())
        args << "-L";
    if(Utils::tarsnapVersionMinimum("1.0.36"))
        args << "--creationtime"
             << QString::number(backupTask->timestamp().toTime_t());
    args << "--quiet"
         << "--print-stats"
         << "--no-humanize-numbers"
         << "-c"
         << "-f" << backupTask->name();
    foreach(QString exclude, backupTask->getExcludesList())
    {
        args << "--exclude" << exclude;
    }
    foreach(QUrl url, backupTask->urls())
    {
        args << url.toLocalFile();
    }
    bTask->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    bTask->setArguments(args);
    backupTask->setCommand(bTask->command() + " " + bTask->arguments().join(" "));
    bTask->setData(backupTask->uuid());
    connect(bTask, &TarsnapTask::finished, this,
            &TaskManager::backupTaskFinished, QUEUED);
    connect(bTask, &TarsnapTask::started, this, &TaskManager::backupTaskStarted,
            QUEUED);
    connect(backupTask.data(), &BackupTask::statusUpdate, this,
            &TaskManager::notifyBackupTaskUpdate, QUEUED);
    backupTask->setStatus(TaskStatus::Queued);
    queueTask(bTask, true);
}

void TaskManager::getArchives()
{
    TarsnapTask *listArchivesTask = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    args << "--list-archives"
         << "-vv";
    listArchivesTask->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    listArchivesTask->setArguments(args);
    listArchivesTask->setTruncateLogOutput(true);
    connect(listArchivesTask, &TarsnapTask::finished, this,
            &TaskManager::getArchiveListFinished, QUEUED);
    connect(listArchivesTask, &TarsnapTask::started, this,
            [&]() { emit message(tr("Updating archives list from remote...")); },
            QUEUED);
    queueTask(listArchivesTask);
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

    TarsnapTask *statsTask = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    args << "--print-stats"
         << "--no-humanize-numbers"
         << "-f" << archive->name();
    statsTask->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    statsTask->setArguments(args);
    statsTask->setData(QVariant::fromValue(archive));
    connect(statsTask, &TarsnapTask::finished, this,
            &TaskManager::getArchiveStatsFinished, QUEUED);
    connect(statsTask, &TarsnapTask::started, this,
            [=]() {
                emit message(tr("Fetching stats for archive <i>%1</i>...")
                                 .arg(archive->name()));
            },
            QUEUED);
    queueTask(statsTask);
}

void TaskManager::getArchiveContents(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    TarsnapTask *contentsTask = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    QSettings settings;
    if(settings.value("tarsnap/preserve_pathnames", DEFAULT_PRESERVE_PATHNAMES)
           .toBool())
        args << "-P";
    args << "-tv"
         << "-f" << archive->name();
    contentsTask->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    contentsTask->setArguments(args);
    contentsTask->setData(QVariant::fromValue(archive));
    contentsTask->setTruncateLogOutput(true);
    connect(contentsTask, &TarsnapTask::finished, this,
            &TaskManager::getArchiveContentsFinished, QUEUED);
    connect(contentsTask, &TarsnapTask::started, this,
            [=]() {
                emit message(tr("Fetching contents for archive <i>%1</i>...")
                                 .arg(archive->name()));
            },
            QUEUED);
    queueTask(contentsTask);
}

void TaskManager::deleteArchives(QList<ArchivePtr> archives)
{
    if(archives.isEmpty())
    {
        DEBUG << "Empty QList<ArchivePtr> passed.";
        return;
    }

    foreach(ArchivePtr archive, archives)
        archive->setDeleteScheduled(true);

    TarsnapTask *delArchives = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    args << "--print-stats"
         << "-d";
    foreach(ArchivePtr archive, archives)
    {
        args << "-f" << archive->name();
    }
    delArchives->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    delArchives->setArguments(args);
    delArchives->setData(QVariant::fromValue(archives));
    connect(delArchives, &TarsnapTask::finished, this,
            &TaskManager::deleteArchivesFinished, QUEUED);
    connect(delArchives, &TarsnapTask::canceled, this,
            [=](QVariant data) {
                QList<ArchivePtr> archives = data.value<QList<ArchivePtr>>();
                foreach(ArchivePtr archive, archives)
                    archive->setDeleteScheduled(false);
            },
            QUEUED);
    connect(delArchives, &TarsnapTask::started, this,
            [=](QVariant data) {
                QList<ArchivePtr> archives = data.value<QList<ArchivePtr>>();
                notifyArchivesDeleted(archives, false);
            },
            QUEUED);
    queueTask(delArchives, true);
}

void TaskManager::getOverallStats()
{
    TarsnapTask *overallStats = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    args << "--print-stats"
         << "--no-humanize-numbers";
    overallStats->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    overallStats->setArguments(args);
    connect(overallStats, &TarsnapTask::finished, this,
            &TaskManager::overallStatsFinished, QUEUED);
    queueTask(overallStats);
}

void TaskManager::fsck(bool prune)
{
    TarsnapTask *fsck = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    if(prune)
        args << "--fsck-prune";
    else
        args << "--fsck";
    fsck->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    fsck->setArguments(args);
    connect(fsck, &TarsnapTask::finished, this, &TaskManager::fsckFinished,
            QUEUED);
    connect(fsck, &TarsnapTask::started, this,
            [=]() { emit message(tr("Cache repair initiated.")); }, QUEUED);
    queueTask(fsck, true);
}

void TaskManager::nuke()
{
    TarsnapTask *nuke = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    args << "--nuke";
    nuke->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    nuke->setStdIn("No Tomorrow\n");
    nuke->setArguments(args);
    connect(nuke, &TarsnapTask::finished, this, &TaskManager::nukeFinished,
            QUEUED);
    connect(nuke, &TarsnapTask::started, this,
            [=]() { emit message(tr("Archives nuke initiated...")); }, QUEUED);
    queueTask(nuke, true);
}

void TaskManager::restoreArchive(ArchivePtr            archive,
                                 ArchiveRestoreOptions options)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    TarsnapTask *restore = new TarsnapTask();
    QStringList  args;
    initTarsnapArgs(args);
    if(options.optionRestore)
    {
        QSettings settings;
        args << "-x"
             << "-P"
             << "-C"
             << settings.value("app/downloads_dir", DEFAULT_DOWNLOADS).toString();
    }
    if(options.optionRestoreDir)
        args << "-x"
             << "-C" << options.path;
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
        restore->setStdOutFile(options.path);
    }
    if(!options.files.isEmpty())
    {
        args << "-T"
             << "-";
        restore->setStdIn(options.files.join(QChar('\n')));
    }
    args << "-f" << archive->name();
    restore->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    restore->setArguments(args);
    restore->setData(QVariant::fromValue(archive));
    connect(restore, &TarsnapTask::finished, this,
            &TaskManager::restoreArchiveFinished, QUEUED);
    connect(restore, &TarsnapTask::started, this,
            [=]() {
                emit message(tr("Restoring from archive <i>%1</i>...")
                                 .arg(archive->name()));
            },
            QUEUED);
    queueTask(restore);
}

void TaskManager::getKeyId(QString key_filename)
{
    QFileInfo keyFile(key_filename);
    if(!keyFile.exists() || !Utils::tarsnapVersionMinimum("1.0.37"))
    {
        DEBUG << "Invalid key path or tarsnap version lower than 1.0.37.";
        return;
    }
    TarsnapTask *keymgmtTask = new TarsnapTask();
    QStringList  args;
    args << "--print-key-id" << key_filename;
    keymgmtTask->setCommand(makeTarsnapCommand(CMD_TARSNAPKEYMGMT));
    keymgmtTask->setArguments(args);
    keymgmtTask->setData(key_filename);
    connect(keymgmtTask, &TarsnapTask::finished, this,
            &TaskManager::getKeyIdFinished, QUEUED);
    queueTask(keymgmtTask);
}

void TaskManager::initializeCache()
{
    QSettings settings;
    QString   tarsnapCacheDir = settings.value("tarsnap/cache").toString();
    QDir      cacheDir(tarsnapCacheDir);
    if(!tarsnapCacheDir.isEmpty()
       && !cacheDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count())
    {
        if(!Utils::tarsnapVersionMinimum("1.0.38"))
        {
            DEBUG << "Tarsnap CLI version 1.0.38 or higher required to use "
                     "--initialize-cachedir.";
            return;
        }
        TarsnapTask *initTask = new TarsnapTask();
        QStringList  args;
        initTarsnapArgs(args);
        args << "--initialize-cachedir";
        initTask->setCommand(makeTarsnapCommand(CMD_TARSNAP));
        initTask->setArguments(args);
        queueTask(initTask);
    }
    else
    {
        fsck(true);
    }
}

void TaskManager::findMatchingArchives(QString jobPrefix)
{
    QList<ArchivePtr> matching;
    foreach(ArchivePtr archive, _archiveMap)
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
    emit    message(title, body);
    emit    displayNotification(title + "\n" + body);
    // Quit with a delay to allow for the system notifications to go through
    QTimer *quitTimer = new QTimer(this);
    quitTimer->setSingleShot(true);
    connect(quitTimer, &QTimer::timeout, qApp, QCoreApplication::quit);
    quitTimer->start(1000);
}

void TaskManager::runScheduledJobs()
{
    loadJobs();
    QSettings settings;
    QDate     now(QDate::currentDate());
    QDate     nextDaily  = settings.value("app/next_daily_timestamp").toDate();
    QDate     nextWeekly = settings.value("app/next_weekly_timestamp").toDate();
    QDate nextMonthly = settings.value("app/next_monthly_timestamp").toDate();
    bool  doDaily     = false;
    bool  doWeekly    = false;
    bool  doMonthly   = false;
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
          << settings.value("app/next_daily_timestamp").toDate().toString();
    DEBUG << "Weekly: " << doWeekly;
    DEBUG << "Next weekly: "
          << settings.value("app/next_weekly_timestamp").toDate().toString();
    DEBUG << "Monthly: " << doWeekly;
    DEBUG << "Next monthly: "
          << settings.value("app/next_monthly_timestamp").toDate().toString();
    bool nothingToDo = true;
    foreach(JobPtr job, _jobMap)
    {
        // Do we need to run any jobs?
        if((doDaily && (job->optionScheduledEnabled() == JobSchedule::Daily))
           || (doWeekly && (job->optionScheduledEnabled() == JobSchedule::Weekly))
           || (doMonthly
               && (job->optionScheduledEnabled() == JobSchedule::Monthly)))
        {
            // Before the first job...
            if(nothingToDo) {
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
                task->cancel();
                task->deleteLater();
            }
        }
        emit message("Cleared queued tasks.");
    }
    if(interrupt)
    {
        if(!_runningTasks.isEmpty())
            _runningTasks.first()->interrupt();
        emit message("Interrupting current backup.");
    }
    if(running)
    {
        foreach(TarsnapTask *task, _runningTasks)
        {
            if(task)
                task->stop();
        }
        emit message("Stopped running tasks.");
    }
}

void TaskManager::backupTaskFinished(QVariant data, int exitCode,
                                     QString stdOut, QString stdErr)
{
    BackupTaskPtr backupTask = _backupTaskMap[data.toUuid()];
    if(!backupTask)
    {
        DEBUG << "Task not found: " << data.toUuid();
        return;
    }
    backupTask->setExitCode(exitCode);
    backupTask->setOutput(stdOut + stdErr);
    bool truncated = false;
    if(exitCode != SUCCESS)
    {
        int lastIndex =
            stdErr.lastIndexOf(QLatin1String("tarsnap: Archive truncated"), -1,
                               Qt::CaseSensitive);
        if(lastIndex == -1)
        {
            backupTask->setStatus(TaskStatus::Failed);
            parseError(stdErr);
            return;
        }
        else
        {
            truncated = true;
        }
    }

    ArchivePtr archive(new Archive);
    archive->setName(backupTask->name());
    if(truncated)
    {
        archive->setName(archive->name().append(".part"));
        archive->setTruncated(true);
    }
    archive->setCommand(backupTask->command());
    // Lose milliseconds precision by converting to Unix timestamp and back.
    // So that a subsequent comparison in getArchiveListFinished won't fail.
    archive->setTimestamp(
        QDateTime::fromTime_t(backupTask->timestamp().toTime_t()));
    archive->setJobRef(backupTask->jobRef());
    parseArchiveStats(stdErr, true, archive);
    archive->save();
    backupTask->setArchive(archive);
    backupTask->setStatus(TaskStatus::Completed);
    _archiveMap.insert(archive->name(), archive);
    foreach(JobPtr job, _jobMap)
    {
        if(job->objectKey() == archive->jobRef())
            emit job->loadArchives();
    }
    emit addArchive(archive);
    parseGlobalStats(stdErr);
}

void TaskManager::backupTaskStarted(QVariant data)
{
    BackupTaskPtr backupTask = _backupTaskMap[data.toString()];
    backupTask->setStatus(TaskStatus::Running);
}

void TaskManager::registerMachineFinished(QVariant data, int exitCode,
                                          QString stdOut, QString stdErr)
{
    Q_UNUSED(data)
    if(exitCode == SUCCESS)
        emit registerMachineStatus(TaskStatus::Completed, stdOut);
    else
        emit registerMachineStatus(TaskStatus::Failed, stdErr);
}

void TaskManager::getArchiveListFinished(QVariant data, int exitCode,
                                         QString stdOut, QString stdErr)
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
    foreach(QString line, lines)
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
                foreach(JobPtr job, _jobMap)
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
    foreach(ArchivePtr archive, _archiveMap)
    {
        archive->purge();
    }
    _archiveMap.clear();
    _archiveMap = _newArchiveMap;
    foreach(JobPtr job, _jobMap)
    {
        emit job->loadArchives();
    }
    getOverallStats();
}

void TaskManager::getArchiveStatsFinished(QVariant data, int exitCode,
                                          QString stdOut, QString stdErr)
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
                                             QString stdOut, QString stdErr)
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
                                         QString stdOut, QString stdErr)
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
        foreach(ArchivePtr archive, archives)
            archive->setDeleteScheduled(false);
        return;
    }

    if(!archives.empty())
    {
        foreach(ArchivePtr archive, archives)
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
                                       QString stdOut, QString stdErr)
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

void TaskManager::fsckFinished(QVariant data, int exitCode, QString stdOut,
                               QString stdErr)
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

void TaskManager::nukeFinished(QVariant data, int exitCode, QString stdOut,
                               QString stdErr)
{
    Q_UNUSED(data)
    if(exitCode == SUCCESS)
    {
        emit message(tr("All archives nuked successfully."), stdOut);
        fsck();
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
                                         QString stdOut, QString stdErr)
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
        emit message(
            tr("Restoring from archive <i>%1</i>... done.").arg(archive->name()));
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
    BackupTaskPtr backupTask = _backupTaskMap[uuid];
    if(!backupTask)
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
        QString msg = tr("Backup <i>%1</i> completed. (%2 new data on Tarsnap)")
                          .arg(backupTask->name())
                          .arg(Utils::humanBytes(
                              backupTask->archive()->sizeUniqueCompressed()));
        emit message(msg, backupTask->archive()->archiveStats());
        emit displayNotification(msg);
        _backupTaskMap.remove(backupTask->uuid());
        break;
    }
    case TaskStatus::Queued:
        emit message(tr("Backup <i>%1</i> queued.").arg(backupTask->name()));
        break;
    case TaskStatus::Running:
    {
        QString msg = tr("Backup <i>%1</i> is running.").arg(backupTask->name());
        emit    message(msg);
        emit    displayNotification(msg);
        break;
    }
    case TaskStatus::Failed:
    {
        QString msg =
            tr("Backup <i>%1</i> failed: %2")
                .arg(backupTask->name())
                .arg(backupTask->output()
                         .section(QChar('\n'), 0, 0, QString::SectionSkipEmpty)
                         .simplified());
        emit message(msg, backupTask->output());
        emit displayNotification(msg);
        _backupTaskMap.remove(backupTask->uuid());
        break;
    }
    case TaskStatus::Paused:
        emit message(tr("Backup <i>%1</i> paused.").arg(backupTask->name()));
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

void TaskManager::getKeyIdFinished(QVariant data, int exitCode, QString stdOut,
                                   QString stdErr)
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
    connect(task, &TarsnapTask::dequeue, this, &TaskManager::dequeueTask,
            QUEUED);
    _runningTasks.append(task);
    task->setAutoDelete(false);
    _threadPool->start(task);
    emit idle(false);
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
}

void TaskManager::parseError(QString tarsnapOutput)
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

void TaskManager::parseGlobalStats(QString tarsnapOutput)
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

void TaskManager::parseArchiveStats(QString tarsnapOutput,
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
    foreach(QString line, lines)
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

QString TaskManager::makeTarsnapCommand(QString cmd)
{
    QSettings settings;
    QString   _tarsnapDir = settings.value("tarsnap/path").toString();
    if(_tarsnapDir.isEmpty())
        return cmd;
    else
        return _tarsnapDir + QDir::separator() + cmd;
}

void TaskManager::initTarsnapArgs(QStringList &args)
{
    QSettings settings;
    QString   tarsnapKeyFile = settings.value("tarsnap/key").toString();
    if(!tarsnapKeyFile.isEmpty())
        args << "--keyfile" << tarsnapKeyFile;
    QString tarsnapCacheDir = settings.value("tarsnap/cache").toString();
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
            job->setName(query.value(query.record().indexOf("name")).toString());
            connect(job.data(), &Job::loadArchives, this,
                    &TaskManager::loadJobArchives, QUEUED);
            job->load();
            _jobMap[job->name()] = job;
        } while(query.next());
    }
    emit jobsList(_jobMap);
}

void TaskManager::deleteJob(JobPtr job, bool purgeArchives)
{
    if(job)
    {
        // Clear JobRef for assigned Archives.
        foreach(ArchivePtr archive, job->archives())
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
    foreach(ArchivePtr archive, _archiveMap)
    {
        if(archive->jobRef() == job->objectKey())
            archives << archive;
    }
    job->setArchives(archives);
}

void TaskManager::getTaskInfo()
{
    bool backupTaskRunning = false;
    if(!_runningTasks.isEmpty() && !_backupTaskMap.isEmpty())
    {
        foreach(TarsnapTask *task, _runningTasks)
        {
            if(task && _backupTaskMap.contains(task->data().toUuid()))
            {
                backupTaskRunning = true;
                break;
            }
        }
    }
    emit taskInfo(backupTaskRunning, _runningTasks.count(), _taskQueue.count());
}

void TaskManager::addJob(JobPtr job)
{
    _jobMap[job->name()] = job;
    connect(job.data(), &Job::loadArchives, this, &TaskManager::loadJobArchives,
            QUEUED);
    emit message(tr("Job <i>%1</i> added.").arg(job->name()));
}

void TaskManager::getTarsnapVersionFinished(QVariant data, int exitCode,
                                            QString stdOut, QString stdErr)
{
    Q_UNUSED(data)

    if(exitCode != SUCCESS)
    {
        emit message(tr("Error: Failed to get Tarsnap version."),
                     tr("Tarsnap exited with code %1 and output:\n%2")
                         .arg(exitCode)
                         .arg(stdErr));
        return;
    }

    QRegExp versionRx("^tarsnap (\\S+)\\s?$");
    if(-1 != versionRx.indexIn(stdOut))
        emit tarsnapVersion(versionRx.cap(1));
}

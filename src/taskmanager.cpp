#include "taskmanager.h"
#include "utils.h"
#include "debug.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>

TaskManager::TaskManager(QObject *parent) : QObject()
  , _threadPool(QThreadPool::globalInstance()), _aggressiveNetworking(false)
  , _preservePathnames(true), _headless(false)
{
    Q_UNUSED(parent);
    // Move the operations belonging to the Task manager to a separate thread
    _managerThread.start();
    moveToThread(&_managerThread);
}

TaskManager::~TaskManager()
{
    _managerThread.quit();
    _managerThread.wait();
}

void TaskManager::loadSettings()
{
    QSettings settings;

    _tarsnapDir             = settings.value("tarsnap/path").toString();
    _tarsnapVersion         = settings.value("tarsnap/version").toString();
    _tarsnapCacheDir        = settings.value("tarsnap/cache").toString();
    _tarsnapKeyFile         = settings.value("tarsnap/key").toString();
    _aggressiveNetworking   = settings.value("tarsnap/aggressive_networking", false).toBool();
    _preservePathnames      = settings.value("tarsnap/preserve_pathnames", true).toBool();

    // First time init of the Store
    PersistentStore::instance();
}

void TaskManager::getTarsnapVersion(QString tarsnapPath)
{
    TarsnapClient *tarsnap = new TarsnapClient();
    tarsnap->setCommand(tarsnapPath + QDir::separator() + CMD_TARSNAP);
    tarsnap->setArguments(QStringList("--version"));
    connect(tarsnap, SIGNAL(finished(QVariant,int,QString))
            , this, SLOT(getTarsnapVersionFinished(QVariant,int,QString))
            , Qt::QueuedConnection);
    queueTask(tarsnap);
}

void TaskManager::registerMachine(QString user, QString password, QString machine, QString key, QString tarsnapPath, QString cachePath)
{
    TarsnapClient *registerClient = new TarsnapClient();
    QStringList args;
    QFileInfo keyFile(key);
    if(keyFile.exists())
    {
        // existing key, just check with a tarsnap --print-stats command
        args << "--fsck"<< "--keyfile"<< key << "--cachedir" << cachePath;
        registerClient->setCommand(tarsnapPath + QDir::separator() + CMD_TARSNAP);
        registerClient->setArguments(args);
    }
    else
    {
        // register machine with tarsnap-keygen
        args << "--user" << user << "--machine" << machine << "--keyfile"<< key;
        registerClient->setCommand(tarsnapPath + QDir::separator() + CMD_TARSNAPKEYGEN);
        registerClient->setArguments(args);
        registerClient->setPassword(password);
        registerClient->setRequiresPassword(true);
    }
    connect(registerClient, SIGNAL(finished(QVariant,int,QString))
            , this, SLOT(registerMachineFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(registerClient);
}

void TaskManager::backupNow(BackupTaskPtr backupTask)
{
    if(backupTask == 0)
    {
        DEBUG << "Null BackupTaskPtr passed.";
        return;
    }
    _backupTaskMap[backupTask->uuid()] = backupTask;
    TarsnapClient *backupClient = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    if(_aggressiveNetworking)
        args << "--aggressive-networking";
    if(backupTask->optionPreservePaths())
        args << "-P";
    if(!backupTask->optionTraverseMount())
        args << "--one-file-system";
    if(backupTask->optionFollowSymLinks())
        args << "-L";
    args << "--quiet" << "--print-stats" << "--no-humanize-numbers" << "-c"
         << "-f" << backupTask->name();
    foreach (QString exclude, backupTask->getExcludesList())
    {
        args << "--exclude" << exclude;
    }
    foreach (QUrl url, backupTask->urls())
    {
        args << url.toLocalFile();
    }
    backupClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    backupClient->setArguments(args);
    backupClient->setData(backupTask->uuid());
    connect(backupClient, SIGNAL(finished(QVariant,int,QString)), this,
            SLOT(backupTaskFinished(QVariant,int,QString)), Qt::QueuedConnection);
    connect(backupClient, SIGNAL(started(QVariant)), this,
            SLOT(backupTaskStarted(QVariant)), Qt::QueuedConnection);
    backupTask->setStatus(TaskStatus::Queued);
    queueTask(backupClient, true);
}

void TaskManager::loadArchives()
{
    // Load from PersistentStore first
    _archiveMap.clear();
    PersistentStore& store = PersistentStore::instance();
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
            archive->setName(query.value(query.record().indexOf("name")).toString());
            archive->load();
            _archiveMap[archive->name()] = archive;
        }while(query.next());
    }
    emit archiveList(_archiveMap.values());

    // Issue sync from remote next
    TarsnapClient *listArchivesClient = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty()) // We shouldn't need to pass this as per the man page, however Tarsnap CLI seems to require it
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--list-archives" << "-vv";
    listArchivesClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    listArchivesClient->setArguments(args);
    connect(listArchivesClient, SIGNAL(finished(QVariant,int,QString))
            , this, SLOT(getArchiveListFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(listArchivesClient);
}

void TaskManager::getArchiveStats(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    _archiveMap.insert(archive->name(), archive);

    TarsnapClient *statsClient = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--print-stats" << "--no-humanize-numbers" << "-f" << archive->name();
    statsClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    statsClient->setArguments(args);
    statsClient->setData(archive->name());
    connect(statsClient, SIGNAL(finished(QVariant,int,QString)), this
            , SLOT(getArchiveStatsFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(statsClient);
}

void TaskManager::getArchiveContents(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    _archiveMap.insert(archive->name(), archive);

    TarsnapClient *contentsClient = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty()) // We shouldn't need to pass this as per the man page, however Tarsnap CLI seems to require it
        args << "--cachedir" << _tarsnapCacheDir;
    if(_preservePathnames)
        args << "-P";
    args << "-t" << "-f" << archive->name();
    contentsClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    contentsClient->setArguments(args);
    contentsClient->setData(archive->name());
    connect(contentsClient, SIGNAL(finished(QVariant,int,QString))
            , this, SLOT(getArchiveContentsFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(contentsClient);
}

void TaskManager::deleteArchives(QList<ArchivePtr> archives)
{
    if(archives.isEmpty())
    {
        DEBUG << "Empty QList<ArchivePtr> passed.";
        return;
    }

    TarsnapClient *delArchives = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--print-stats" << "-d";
    foreach (ArchivePtr archive, archives) {
        args << "-f" << archive->name();
    }
    delArchives->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    delArchives->setArguments(args);
    delArchives->setData(QVariant::fromValue(archives));
    connect(delArchives, SIGNAL(finished(QVariant,int,QString))
            , this, SLOT(deleteArchivesFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(delArchives, true);
}

void TaskManager::getOverallStats()
{
    TarsnapClient *overallStats = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--print-stats" << "--no-humanize-numbers";
    overallStats->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    overallStats->setArguments(args);
    connect(overallStats, SIGNAL(finished(QVariant,int,QString))
            , this, SLOT(overallStatsFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(overallStats);
}

void TaskManager::fsck()
{
    TarsnapClient *fsck = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--fsck-prune";
    fsck->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    fsck->setArguments(args);
    connect(fsck, SIGNAL(finished(QVariant,int,QString)), this
            , SLOT(fsckFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(fsck, true);
}

void TaskManager::nuke()
{
    TarsnapClient *nuke = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--nuke";
    nuke->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    nuke->setPassword("No Tomorrow");
    nuke->setRequiresPassword(true);
    nuke->setArguments(args);
    connect(nuke, SIGNAL(finished(QVariant,int,QString)), this
            , SLOT(nukeFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(nuke, true);
}

void TaskManager::restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    _archiveMap.insert(archive->name(), archive);

    TarsnapClient *restore = new TarsnapClient();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(options.preservePaths)
        args << "-P";
    if(!options.chdir.isEmpty())
        args << "-C" << options.chdir;
    if(!options.overwriteFiles)
        args << "-k";
    if(options.keepNewerFiles)
        args << "--keep-newer-files";
    args << "-x" << "-f" << archive->name();
    restore->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    restore->setArguments(args);
    restore->setData(archive->name());
    connect(restore, SIGNAL(finished(QVariant,int,QString)), this
            , SLOT(restoreArchiveFinished(QVariant,int,QString)), Qt::QueuedConnection);
    queueTask(restore);
}

void TaskManager::runScheduledJobs()
{
    loadJobs();
    bool nothingToDo = true;
    foreach(JobPtr job, _jobMap)
    {
        if(job->optionScheduledEnabled())
        {
            backupNow(job->createBackupTask());
            nothingToDo = false;
        }
    }
    if(nothingToDo)
        qApp->quit();
    else
        _headless = true;
}

void TaskManager::stopTasks()
{
    foreach (TarsnapClient *client, _runningTasks)
    {
        if(client)
            client->stop();
    }
}

void TaskManager::backupTaskFinished(QVariant data, int exitCode, QString output)
{
    BackupTaskPtr backupTask = _backupTaskMap[data.toString()];
    backupTask->setExitCode(exitCode);
    backupTask->setOutput(output);
    if(exitCode == 0)
    {
        ArchivePtr archive(new Archive);
        archive->setName(backupTask->name());
        archive->setCommand(qobject_cast<TarsnapClient*>(sender())->command() + " " + qobject_cast<TarsnapClient*>(sender())->arguments().join(" "));
        //TODO: set timestamp to tarsnap timestamp when possible
        archive->setTimestamp(QDateTime::currentDateTime());
        archive->setJobRef(backupTask->jobRef());
        parseArchiveStats(output, true, archive);
        backupTask->setArchive(archive);
        backupTask->setStatus(TaskStatus::Completed);
        _archiveMap.insert(archive->name(), archive);
        emit archiveList(_archiveMap.values());
        parseGlobalStats(output);
    }
    else
    {
        backupTask->setStatus(TaskStatus::Failed);
    }
    _backupTaskMap.take(backupTask->uuid());
    if(_headless && _backupTaskMap.isEmpty())
    {
        qApp->quit();
    }
}

void TaskManager::backupTaskStarted(QVariant data)
{
    BackupTaskPtr backupTask = _backupTaskMap[data.toString()];
    backupTask->setStatus(TaskStatus::Running);
}

void TaskManager::registerMachineFinished(QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data)
    if(exitCode == 0)
        emit registerMachineStatus(TaskStatus::Completed, output);
    else
        emit registerMachineStatus(TaskStatus::Failed, output);
}

void TaskManager::getArchiveListFinished(QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data)
    if(exitCode == 0)
    {
        QMap<QString, ArchivePtr>   _newArchiveMap;
        QStringList lines = output.trimmed().split('\n');
        foreach (QString line, lines)
        {
            QRegExp archiveDetailsRX("^(.+)\\t+(\\S+\\s+\\S+)\\t+(.+)$");
            if(-1 != archiveDetailsRX.indexIn(line))
            {
                QStringList archiveDetails = archiveDetailsRX.capturedTexts();
                archiveDetails.removeFirst();
                QDateTime timestamp = QDateTime::fromString(archiveDetails[1], Qt::ISODate);
                ArchivePtr archive(new Archive);
                bool update = false;
                archive->setName(archiveDetails[0]);
                archive->load();
                if(archive->objectKey().isEmpty())
                {
                    update = true;
                }
                else if(archive->timestamp() != timestamp)
                {
                    //TODO: Remove jobRef carryon when I have a way of getting a tarsnap timestamp
                    //precisely after a backup has completed
                    QString jobRef = archive->jobRef();
                    archive->purge();
                    archive.clear();
                    archive = archive.create();
                    archive->setName(archiveDetails[0]);
                    archive->setJobRef(jobRef);
                    update = true;
                }
                if(update)
                {
                    archive->setTimestamp(timestamp);
                    archive->setCommand(archiveDetails[2]);
                    archive->save();
                    getArchiveStats(archive);
                }
                _newArchiveMap.insert(archive->name(), archive);
                _archiveMap.remove(archive->name());
            }
        }
        // Purge archives left in old _archiveMap (not mirrored by the remote)
        foreach (ArchivePtr archive, _archiveMap)
        {
            archive->purge();
        }
        _archiveMap.clear();
        _archiveMap = _newArchiveMap;
        emit archiveList(_archiveMap.values(), true);
        getOverallStats();
    }
}

void TaskManager::getArchiveStatsFinished(QVariant data, int exitCode, QString output)
{
    ArchivePtr archive = _archiveMap[data.toString()];
    if(archive && (exitCode == 0))
    {
        parseArchiveStats(output, false, archive);
        parseGlobalStats(output);
    }
}

void TaskManager::getArchiveContentsFinished(QVariant data, int exitCode, QString output)
{
    ArchivePtr archive = _archiveMap[data.toString()];
    if(archive && (exitCode == 0))
    {
        archive->setContents(output.trimmed().split('\n', QString::SkipEmptyParts));
        archive->save();
    }
}

void TaskManager::deleteArchivesFinished(QVariant data, int exitCode, QString output)
{

    if(exitCode == 0)
    {
        QList<ArchivePtr> archives = data.value<QList<ArchivePtr>>();
        if(!archives.empty())
        {
            foreach (ArchivePtr archive, archives) {
                _archiveMap.remove(archive->name());
                archive->purge();
            }
            emit archiveList(_archiveMap.values());
            emit archivesDeleted(archives);
        }
        parseGlobalStats(output);
    }
}

void TaskManager::overallStatsFinished(QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data);

    if(exitCode == 0)
    {
        parseGlobalStats(output);
    }
}

void TaskManager::fsckFinished(QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data)
    if(exitCode == 0)
        emit fsckStatus(TaskStatus::Completed, output);
    else
        emit fsckStatus(TaskStatus::Failed, output);
}

void TaskManager::nukeFinished(QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data)
    if(exitCode == 0)
        emit nukeStatus(TaskStatus::Completed, output);
    else
        emit nukeStatus(TaskStatus::Failed, output);
    fsck();
    loadArchives();
}

void TaskManager::restoreArchiveFinished(QVariant data, int exitCode, QString output)
{

    ArchivePtr archive = _archiveMap[data.toString()];
    if(archive && (exitCode == 0))
        emit restoreArchiveStatus(archive, TaskStatus::Completed, output);
    else
        emit restoreArchiveStatus(archive, TaskStatus::Failed, output);
}

void TaskManager::queueTask(TarsnapClient *cli, bool exclusive)
{
    if(cli == NULL)
    {
        DEBUG << "NULL argument";
        return;
    }
    if(exclusive && !_runningTasks.isEmpty())
        _taskQueue.enqueue(cli);
    else
        startTask(cli);
}

void TaskManager::startTask(TarsnapClient *cli)
{
    if(cli == NULL)
    {
        if(!_taskQueue.isEmpty())
            cli = _taskQueue.dequeue();
        else
            return;
    }
    connect(cli, SIGNAL(terminated(QVariant)), this , SLOT(dequeueTask()),
            Qt::QueuedConnection);
    _runningTasks.append(cli);
    cli->setAutoDelete(false);
    _threadPool->start(cli);
    emit idle(false);
}

void TaskManager::dequeueTask()
{
    _runningTasks.removeOne(qobject_cast<TarsnapClient*>(sender()));
    if(_runningTasks.count() == 0)
    {
        if(_taskQueue.isEmpty())
            emit idle(true);
        else
            startTask(NULL);
    }
}

void TaskManager::parseGlobalStats(QString tarsnapOutput)
{
    quint64 sizeTotal;
    quint64 sizeCompressed;
    quint64 sizeUniqueTotal;
    quint64 sizeUniqueCompressed;

    QStringList lines = tarsnapOutput.trimmed().split('\n', QString::SkipEmptyParts);
    if(lines.count() < 3)
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    QRegExp sizeRX("^All archives\\s+(\\d+)\\s+(\\d+)$");
    QRegExp uniqueSizeRX("^\\s+\\(unique data\\)\\s+(\\d+)\\s+(\\d+)$");
    if(-1 != sizeRX.indexIn(lines[1]))
    {
        QStringList captured = sizeRX.capturedTexts();
        captured.removeFirst();
        sizeTotal = captured[0].toLongLong();
        sizeCompressed = captured[1].toLongLong();
    }
    else
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    if(-1 != uniqueSizeRX.indexIn(lines[2]))
    {
        QStringList captured = uniqueSizeRX.capturedTexts();
        captured.removeFirst();
        sizeUniqueTotal = captured[0].toLongLong();
        sizeUniqueCompressed = captured[1].toLongLong();
    }
    else
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    emit overallStats(sizeTotal, sizeCompressed, sizeUniqueTotal, sizeUniqueCompressed
                      , _archiveMap.count());
}

void TaskManager::parseArchiveStats(QString tarsnapOutput, bool newArchiveOutput, ArchivePtr archive)
{
    QStringList lines = tarsnapOutput.trimmed().split('\n', QString::SkipEmptyParts);
    if(lines.count() != 5)
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    QString sizeLine = lines[3];
    QString uniqueSizeLine = lines[4];
    QRegExp sizeRX;
    QRegExp uniqueSizeRX;
    if(newArchiveOutput)
    {
        sizeRX.setPattern("^This archive\\s+(\\d+)\\s+(\\d+)$");
        uniqueSizeRX.setPattern("^New data\\s+(\\d+)\\s+(\\d+)$");
    }
    else
    {
        sizeRX.setPattern(QString("^%1\\s+(\\d+)\\s+(\\d+)$").arg(archive->name()));
        uniqueSizeRX.setPattern("^\\s+\\(unique data\\)\\s+(\\d+)\\s+(\\d+)$");
    }
    if(-1 != sizeRX.indexIn(sizeLine))
    {
        QStringList captured = sizeRX.capturedTexts();
        captured.removeFirst();
        archive->setSizeTotal(captured[0].toLongLong());
        archive->setSizeCompressed(captured[1].toLongLong());
    }
    else
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    if(-1 != uniqueSizeRX.indexIn(uniqueSizeLine))
    {
        QStringList captured = uniqueSizeRX.capturedTexts();
        captured.removeFirst();
        archive->setSizeUniqueTotal(captured[0].toLongLong());
        archive->setSizeUniqueCompressed(captured[1].toLongLong());
    }
    else
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    archive->save();
}

QString TaskManager::makeTarsnapCommand(QString cmd)
{
    if(_tarsnapDir.isEmpty())
        return cmd;
    else
        return _tarsnapDir + QDir::separator() + cmd;
}

void TaskManager::loadJobs()
{
    _jobMap.clear();
    PersistentStore& store = PersistentStore::instance();
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
            connect(job.data(), SIGNAL(loadArchives()), this, SLOT(loadJobArchives()), Qt::QueuedConnection);
            job->load();
            _jobMap[job->name()] = job;
        }while(query.next());
    }
    emit jobsList(_jobMap);
}

void TaskManager::deleteJob(JobPtr job, bool purgeArchives)
{
    if(job)
    {
        if(purgeArchives)
            deleteArchives(job->archives());
        job->purge();
        _jobMap.remove(job->name());
    }
}

void TaskManager::loadJobArchives()
{
    Job *job = qobject_cast<Job*>(sender());
    QList<ArchivePtr> archives;
    foreach (ArchivePtr archive, _archiveMap)
    {
        if(archive->jobRef() == job->objectKey())
        {
            archives << archive;
        }
    }
    job->setArchives(archives);
}

void TaskManager::getTarsnapVersionFinished(QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data)
    if(exitCode == 0)
    {
        QRegExp versionRx("^tarsnap (\\S+)\\s$");
        if(-1 != versionRx.indexIn(output))
            emit tarsnapVersion(versionRx.cap(1));
    }
}

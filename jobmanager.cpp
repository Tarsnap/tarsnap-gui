#include "jobmanager.h"
#include "debug.h"

#include <QFileInfo>
#include <QDir>
#include <QSettings>

JobManager::JobManager(QObject *parent) : QObject()
  , _threadPool(QThreadPool::globalInstance()), _aggressiveNetworking(false)
  , _preservePathnames(true)
{
    Q_UNUSED(parent);
    // Move the operations belonging to the Job manager to a separate thread
    _managerThread.start();
    moveToThread(&_managerThread);
}

JobManager::~JobManager()
{
    _managerThread.quit();
    _managerThread.wait();
}

void JobManager::loadSettings()
{
    QSettings settings;
    _tarsnapDir      = settings.value("tarsnap/path").toString();
    _tarsnapCacheDir = settings.value("tarsnap/cache").toString();
    _tarsnapKeyFile  = settings.value("tarsnap/key").toString();
    _aggressiveNetworking = settings.value("tarsnap/aggressive_networking", false).toBool();
    _preservePathnames = settings.value("tarsnap/preserve_pathnames", true).toBool();
}

void JobManager::registerMachine(QString user, QString password, QString machine, QString key, QString tarsnapPath, QString cachePath)
{
    TarsnapCLI *registerClient = new TarsnapCLI();
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
    connect(registerClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString))
            , this, SLOT(registerMachineFinished(QUuid,QVariant,int,QString)));
    queueTask(registerClient);
}

void JobManager::backupNow(BackupTaskPtr backupTask)
{
    if(backupTask.isNull())
    {
        DEBUG << "Null BackupTaskPtr passed.";
        return;
    }
    _backupTaskMap[backupTask->uuid] = backupTask;
    TarsnapCLI *backupClient = new TarsnapCLI(backupTask->uuid);
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    if(_aggressiveNetworking)
        args << "--aggressive-networking";
    if(_preservePathnames)
        args << "-P";
    args << "--quiet" << "-c" << "--print-stats" << "-f" << backupTask->name;
    foreach (QUrl url, backupTask->urls) {
        args << url.toLocalFile();
    }
    backupClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    backupClient->setArguments(args);
    connect(backupClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString))
            , this, SLOT(backupTaskFinished(QUuid,QVariant,int,QString)));
    connect(backupClient, SIGNAL(clientStarted(QUuid)), this, SLOT(backupTaskStarted(QUuid)));
    queueTask(backupClient, true);
    backupTask->status = TaskStatus::Queued;
    emit backupTaskUpdate(backupTask);
}

void JobManager::getArchivesList()
{
    TarsnapCLI *listArchivesClient = new TarsnapCLI();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--list-archives" << "-vv";
    listArchivesClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    listArchivesClient->setArguments(args);
    connect(listArchivesClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString))
            , this, SLOT(getArchivesFinished(QUuid,QVariant,int,QString)));
    queueTask(listArchivesClient);
}

void JobManager::getArchiveStats(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    if(!_archiveMap.contains(archive->uuid))
        _archiveMap[archive->uuid] = archive;

    TarsnapCLI *statsClient = new TarsnapCLI(archive->uuid);
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--print-stats" << "-f" << archive->name;
    statsClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    statsClient->setArguments(args);
    connect(statsClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this
            , SLOT(getArchiveStatsFinished(QUuid,QVariant,int,QString)));
    queueTask(statsClient);
}

void JobManager::getArchiveContents(ArchivePtr archive)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    if(!_archiveMap.contains(archive->uuid))
        _archiveMap[archive->uuid] = archive;

    TarsnapCLI *contentsClient = new TarsnapCLI(archive->uuid);
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(_preservePathnames)
        args << "-P";
    args << "-t" << "-f" << archive->name;
    contentsClient->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    contentsClient->setArguments(args);
    connect(contentsClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString))
            , this, SLOT(getArchiveContentsFinished(QUuid,QVariant,int,QString)));
    queueTask(contentsClient);
}

void JobManager::deleteArchives(QList<ArchivePtr> archives)
{
    if(archives.isEmpty())
    {
        DEBUG << "Empty QList<ArchivePtr> passed.";
        return;
    }

    TarsnapCLI *delArchives = new TarsnapCLI();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "-d";
    foreach (ArchivePtr archive, archives) {
        args << "-f" << archive->name;
    }
    delArchives->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    delArchives->setArguments(args);
    delArchives->setData(QVariant::fromValue(archives));
    connect(delArchives, SIGNAL(clientFinished(QUuid,QVariant,int,QString))
            , this, SLOT(deleteArchiveFinished(QUuid,QVariant,int,QString)));
    queueTask(delArchives, true);
}

void JobManager::getOverallStats()
{
    TarsnapCLI *overallStats = new TarsnapCLI();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--print-stats";
    overallStats->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    overallStats->setArguments(args);
    connect(overallStats, SIGNAL(clientFinished(QUuid,QVariant,int,QString))
            , this, SLOT(overallStatsFinished(QUuid,QVariant,int,QString)));
    queueTask(overallStats);
}

void JobManager::runFsck()
{
    TarsnapCLI *fsck = new TarsnapCLI();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--fsck-prune";
    fsck->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    fsck->setArguments(args);
    connect(fsck, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this
            , SLOT(fsckFinished(QUuid,QVariant,int,QString)));
    queueTask(fsck);
}

void JobManager::nukeArchives()
{
    TarsnapCLI *nuke = new TarsnapCLI();
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
    connect(nuke, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this
            , SLOT(nukeFinished(QUuid,QVariant,int,QString)));
    queueTask(nuke);
}

void JobManager::restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options)
{
    if(archive.isNull())
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    if(!_archiveMap.contains(archive->uuid))
        _archiveMap[archive->uuid] = archive;

    TarsnapCLI *restore = new TarsnapCLI(archive->uuid);
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
    args << "-x" << "-f" << archive->name;
    restore->setCommand(makeTarsnapCommand(CMD_TARSNAP));
    restore->setArguments(args);
    connect(restore, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this
            , SLOT(restoreArchiveFinished(QUuid,QVariant,int,QString)));
    queueTask(restore);
}

void JobManager::backupTaskFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data);
    BackupTaskPtr backupTask = _backupTaskMap[uuid];
    backupTask->exitCode = exitCode;
    backupTask->output = output;
    if(exitCode == 0)
    {
        backupTask->status = TaskStatus::Completed;
        ArchivePtr archive(new Archive);
        archive->name = backupTask->name;
        archive->timestamp = QDateTime::currentDateTime();
        _archiveMap[archive->uuid] = archive;
        parseArchiveStats(output, true, archive);
        backupTask->archive = archive;
        emit archivesList(_archiveMap.values());
    }
    else
    {
        backupTask->status = TaskStatus::Failed;
    }
    emit backupTaskUpdate(backupTask);
    _backupTaskMap.remove(backupTask->uuid);
}

void JobManager::backupTaskStarted(QUuid uuid)
{
    BackupTaskPtr backupTask = _backupTaskMap[uuid];
    backupTask->status = TaskStatus::Running;
    emit backupTaskUpdate(backupTask);
}

void JobManager::registerMachineFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid); Q_UNUSED(data)
    if(exitCode == 0)
        emit registerMachineStatus(TaskStatus::Completed, output);
    else
        emit registerMachineStatus(TaskStatus::Failed, output);
}

void JobManager::getArchivesFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid); Q_UNUSED(data)
    _archiveMap.clear();
    QList<ArchivePtr> archives;
    if(exitCode == 0)
    {
        QStringList lines = output.trimmed().split('\n');
        foreach (QString line, lines) {
            QRegExp archiveDetailsRX("^(\\S+)\\s+(\\S+\\s+\\S+)\\s+(.+)$");
            if(-1 != archiveDetailsRX.indexIn(line))
            {
                QStringList archiveDetails = archiveDetailsRX.capturedTexts();
                archiveDetails.removeFirst();
                ArchivePtr archive(new Archive);
                archive->name = archiveDetails[0];
                archive->timestamp = QDateTime::fromString(archiveDetails[1], Qt::ISODate);
                archive->command = archiveDetails[2];
                archives.append(archive);
                _archiveMap[archive->uuid] = archive;
                getArchiveStats(archive);
            }
        }
        emit archivesList(archives);
    }
}

void JobManager::getArchiveStatsFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data)
    ArchivePtr archive = _archiveMap[uuid];
    if(archive.isNull())
    {
        DEBUG << "Task uuid not found in _archiveMap.";
        return;
    }
    if(exitCode == 0)
    {
        parseArchiveStats(output, false, archive);
    }
}

void JobManager::getArchiveContentsFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data)
    ArchivePtr archive = _archiveMap[uuid];
    if(archive.isNull())
    {
        DEBUG << "Task uuid not found in _archiveMap.";
        return;
    }
    if(exitCode == 0)
    {
        archive->contents = output.trimmed().split('\n', QString::SkipEmptyParts);
        archive->notifyChanged();
    }
}

void JobManager::deleteArchiveFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid);Q_UNUSED(output);
    if(exitCode == 0)
    {
        QList<ArchivePtr> archives = data.value<QList<ArchivePtr>>();
        if(!archives.empty())
        {
            foreach (ArchivePtr archive, archives) {
                _archiveMap.remove(archive->uuid);
            }
            emit archivesDeleted(archives);
        }
    }
}

void JobManager::overallStatsFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid);Q_UNUSED(data);

    qint64      sizeTotal;
    qint64      sizeCompressed;
    qint64      sizeUniqueTotal;
    qint64      sizeUniqueCompressed;

    if(exitCode == 0)
    {
        QStringList lines = output.trimmed().split('\n', QString::SkipEmptyParts);
        if(lines.count() != 3)
        {
            DEBUG << "Malformed output from tarsnap CLI:\n" << output;
            return;
        }
        QRegExp sizeRX("^All archives\\s+(\\S+)\\s+(\\S+)$");
        QRegExp uniqueSizeRX("^\\s+\\(unique data\\)\\s+(\\S+)\\s+(\\S+)$");
        if(-1 != sizeRX.indexIn(lines[1]))
        {
            QStringList captured = sizeRX.capturedTexts();
            captured.removeFirst();
            sizeTotal = captured[0].toLongLong();
            sizeCompressed = captured[1].toLongLong();
        }
        else
        {
            DEBUG << "Malformed output from tarsnap CLI:\n" << output;
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
            DEBUG << "Malformed output from tarsnap CLI:\n" << output;
            return;
        }
        emit overallStats(sizeTotal, sizeCompressed, sizeUniqueTotal, sizeUniqueCompressed
                          , _archiveMap.count(), 0.0f, tr("Normal"));
    }
}

void JobManager::fsckFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid); Q_UNUSED(data)
    if(exitCode == 0)
        emit fsckStatus(TaskStatus::Completed, output);
    else
        emit fsckStatus(TaskStatus::Failed, output);
}

void JobManager::nukeFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid); Q_UNUSED(data)
    if(exitCode == 0)
        emit nukeStatus(TaskStatus::Completed, output);
    else
        emit nukeStatus(TaskStatus::Failed, output);
}

void JobManager::restoreArchiveFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid); Q_UNUSED(data)
    ArchivePtr archive = _archiveMap[uuid];
    if(archive.isNull())
    {
        DEBUG << "Task uuid not found in _archiveMap.";
        return;
    }
    if(exitCode == 0)
        emit restoreArchiveStatus(archive, TaskStatus::Completed, output);
    else
        emit restoreArchiveStatus(archive, TaskStatus::Failed, output);
}

void JobManager::queueTask(TarsnapCLI *cli, bool exclusive)
{
    if(cli == NULL)
    {
        DEBUG << "NULL argument";
        return;
    }
    if(exclusive && !_taskMap.isEmpty())
        _taskQueue.enqueue(cli);
    else
        startTask(cli);
}

void JobManager::startTask(TarsnapCLI *cli)
{
    if(cli == NULL)
    {
        if(!_taskQueue.isEmpty())
            cli = _taskQueue.dequeue();
        else
            return;
    }
    connect(cli, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this
            , SLOT(dequeueTask(QUuid,QVariant,int,QString)));
    _taskMap[cli->uuid()] = cli;
    cli->setAutoDelete(false);
    _threadPool->start(cli);
    emit idle(false);
}

void JobManager::dequeueTask(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(exitCode); Q_UNUSED(output); Q_UNUSED(data)
    _taskMap.remove(uuid);
    if(_taskMap.count() == 0)
    {
        if( _taskQueue.isEmpty())
            emit idle(true);
        else
            startTask(NULL);
    }
}

void JobManager::parseArchiveStats(QString tarsnapOutput, bool newArchiveOutput, ArchivePtr archive)
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
        sizeRX.setPattern("^This archive\\s+(\\S+)\\s+(\\S+)$");
        uniqueSizeRX.setPattern("^New data\\s+(\\S+)\\s+(\\S+)$");
    }
    else
    {
        sizeRX.setPattern("^\\S+\\s+(\\S+)\\s+(\\S+)$");
        uniqueSizeRX.setPattern("^\\s+\\(unique data\\)\\s+(\\S+)\\s+(\\S+)$");
    }
    if(-1 != sizeRX.indexIn(sizeLine))
    {
        QStringList captured = sizeRX.capturedTexts();
        captured.removeFirst();
        archive->sizeTotal = captured[0].toLongLong();
        archive->sizeCompressed = captured[1].toLongLong();
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
        archive->sizeUniqueTotal = captured[0].toLongLong();
        archive->sizeUniqueCompressed = captured[1].toLongLong();
    }
    else
    {
        DEBUG << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    archive->notifyChanged();
}

QString JobManager::makeTarsnapCommand(QString cmd)
{
    if(_tarsnapDir.isEmpty())
        return cmd;
    else
        return _tarsnapDir + QDir::separator() + cmd;
}


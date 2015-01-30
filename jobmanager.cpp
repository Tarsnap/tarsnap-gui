#include "jobmanager.h"

#include <QDebug>
#include <QFileInfo>
#include <QSettings>

JobManager::JobManager(QObject *parent) : QObject(), _threadPool(QThreadPool::globalInstance())
{
    Q_UNUSED(parent)
    // Move the operations belonging to the Job manager to a separate thread
    _managerThread.start();
    moveToThread(&_managerThread);
}

JobManager::~JobManager()
{
    _managerThread.quit();
    _managerThread.wait();
}

void JobManager::reloadSettings()
{
    QSettings settings;
    _tarsnapDir      = settings.value("tarsnap/path").toString();
    _tarsnapCacheDir = settings.value("tarsnap/cache").toString();
    _tarsnapKeyFile  = settings.value("tarsnap/key").toString();
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
        registerClient->setCommand(tarsnapPath + CMD_TARSNAP);
        registerClient->setArguments(args);
    }
    else
    {
        // register machine with tarsnap-keygen
        args << "--user" << user << "--machine" << machine << "--keyfile"<< key;
        registerClient->setCommand(tarsnapPath + CMD_TARSNAPKEYGEN);
        registerClient->setArguments(args);
        registerClient->setPassword(password);
        registerClient->setRequiresPassword(true);
    }
    connect(registerClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(registerMachineFinished(QUuid,int,QString)));
    queueJob(registerClient);
}

void JobManager::backupNow(BackupJobPtr job)
{
    if(job.isNull())
    {
        qDebug() << "Null BackupJobPtr passed.";
        return;
    }
    _backupJobMap[job->uuid] = job;
    TarsnapCLI *backupClient = new TarsnapCLI(job->uuid);
    QStringList args;
    args << "-c" << "-f" << job->name;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    foreach (QUrl url, job->urls) {
        args << url.toLocalFile();
    }
    backupClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    backupClient->setArguments(args);
    connect(backupClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(backupJobFinished(QUuid,int,QString)));
    connect(backupClient, SIGNAL(clientStarted(QUuid)), this, SLOT(backupJobStarted(QUuid)));
    queueJob(backupClient);
    job->status = JobStatus::Started;
    emit backupJobUpdate(job);
}

void JobManager::getArchivesList()
{
    TarsnapCLI *listArchivesClient = new TarsnapCLI();
    QStringList args;
    args << "--list-archives" << "-vv";
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    listArchivesClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    listArchivesClient->setArguments(args);
    connect(listArchivesClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(getArchivesFinished(QUuid,int,QString)));
    queueJob(listArchivesClient);
}

void JobManager::getArchiveStats(ArchivePtr archive)
{
    if(archive.isNull())
    {
        qDebug() << "Null ArchivePtr passed.";
        return;
    }

    if(!_archiveMap.contains(archive->uuid))
        _archiveMap[archive->uuid] = archive;

    TarsnapCLI *statsClient = new TarsnapCLI(archive->uuid);
    QStringList args;
    args << "--print-stats" << "-f" << archive->name;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    statsClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    statsClient->setArguments(args);
    connect(statsClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(getArchiveStatsFinished(QUuid,int,QString)));
    queueJob(statsClient);
}

void JobManager::getArchiveContents(ArchivePtr archive)
{
    if(archive.isNull())
    {
        qDebug() << "Null ArchivePtr passed.";
        return;
    }

    if(!_archiveMap.contains(archive->uuid))
        _archiveMap[archive->uuid] = archive;

    TarsnapCLI *contentsClient = new TarsnapCLI(archive->uuid);
    QStringList args;
    args << "-t" << "-f" << archive->name;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    contentsClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    contentsClient->setArguments(args);
    connect(contentsClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(getArchiveContentsFinished(QUuid,int,QString)));
    queueJob(contentsClient);
}

void JobManager::deleteArchive(ArchivePtr archive)
{
    if(archive.isNull())
    {
        qDebug() << "Null ArchivePtr passed.";
        return;
    }

    TarsnapCLI *delArchive = new TarsnapCLI(archive->uuid);
    QStringList args;
    args << "-d" << "-f" << archive->name;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    delArchive->setCommand(_tarsnapDir + CMD_TARSNAP);
    delArchive->setArguments(args);
    connect(delArchive, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(deleteArchiveFinished(QUuid,int,QString)));
    queueJob(delArchive);
}

void JobManager::backupJobFinished(QUuid uuid, int exitCode, QString output)
{
    BackupJobPtr job = _backupJobMap[uuid];
    job->exitCode = exitCode;
    job->output = output;
//    job->reason = message;
    if(exitCode == 0)
        job->status = JobStatus::Completed;
    else
        job->status = JobStatus::Failed;
    emit backupJobUpdate(job);
    _backupJobMap.remove(job->uuid);
}

void JobManager::backupJobStarted(QUuid uuid)
{
    BackupJobPtr job = _backupJobMap[uuid];
    job->status = JobStatus::Running;
    emit backupJobUpdate(job);
}

void JobManager::registerMachineFinished(QUuid uuid, int exitCode, QString output)
{
    Q_UNUSED(uuid)
    if(exitCode == 0)
        emit registerMachineStatus(JobStatus::Completed, output);
    else
        emit registerMachineStatus(JobStatus::Failed, output);
}

void JobManager::getArchivesFinished(QUuid uuid, int exitCode, QString output)
{
    Q_UNUSED(uuid)
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

void JobManager::getArchiveStatsFinished(QUuid uuid, int exitCode, QString output)
{
    ArchivePtr archive = _archiveMap[uuid];
    if(archive.isNull())
    {
        qDebug() << "uuid not found in _archiveMap.";
        return;
    }
    if(exitCode == 0)
    {
        QStringList lines = output.trimmed().split('\n', QString::SkipEmptyParts);
        if(lines.count() != 5)
        {
            qDebug() << "Malformed output from tarsnap cli: " << ::endl << output;
            return;
        }
        QString sizeLine = lines[3];
        QString uniqueSizeLine = lines[4];
        QRegExp sizeRX("^\\S+\\s+(\\S+)\\s+(\\S+)$");
        QRegExp uniqueSizeRX("^\\s+\\(unique data\\)\\s+(\\S+)\\s+(\\S+)$");
        if(-1 != sizeRX.indexIn(sizeLine))
        {
            QStringList captured = sizeRX.capturedTexts();
            captured.removeFirst();
            archive->sizeTotal = captured[0].toLongLong();
            archive->sizeCompressed = captured[1].toLongLong();
        }
        else
        {
            qDebug() << "Malformed output from tarsnap cli: " << ::endl << output;
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
            qDebug() << "Malformed output from tarsnap cli: " << ::endl << output;
            return;
        }
        archive->notifyChanged();
    }
}

void JobManager::getArchiveContentsFinished(QUuid uuid, int exitCode, QString output)
{
    ArchivePtr archive = _archiveMap[uuid];
    if(archive.isNull())
    {
        qDebug() << "uuid not found in _archiveMap.";
        return;
    }
    if(exitCode == 0)
    {
        archive->contents = output.trimmed().split('\n', QString::SkipEmptyParts);
        archive->notifyChanged();
    }
}

void JobManager::deleteArchiveFinished(QUuid uuid, int exitCode, QString output)
{
    Q_UNUSED(output)
    if(exitCode == 0)
    {
        ArchivePtr archive = _archiveMap.take(uuid);
        emit archiveDeleted(archive);
    }
}

void JobManager::queueJob(TarsnapCLI *cli)
{
    if(cli == NULL)
    {
        qDebug() << "NULL argument";
        return;
    }
    connect(cli, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(dequeueJob(QUuid,int,QString)));
    _jobMap[cli->uuid()] = cli;
    cli->setAutoDelete(false);
    _threadPool->start(cli);
    if(_jobMap.count() == 1)
        emit idle(false);
}

void JobManager::dequeueJob(QUuid uuid, int exitCode, QString output)
{
    Q_UNUSED(exitCode); Q_UNUSED(output)
    _jobMap.remove(uuid);
    if(_jobMap.count() == 0)
        emit idle(true);
}


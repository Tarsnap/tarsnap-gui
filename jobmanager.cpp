#include "jobmanager.h"

#include <QDebug>
#include <QFileInfo>
#include <QSettings>

JobManager::JobManager(QObject *parent) : QObject(), _threadPool(QThreadPool::globalInstance())
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
    connect(registerClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this, SLOT(registerMachineFinished(QUuid,QVariant,int,QString)));
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
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--quiet" << "-c" << "--print-stats" << "-f" << job->name;
    foreach (QUrl url, job->urls) {
        args << url.toLocalFile();
    }
    backupClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    backupClient->setArguments(args);
    connect(backupClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this, SLOT(backupJobFinished(QUuid,QVariant,int,QString)));
    connect(backupClient, SIGNAL(clientStarted(QUuid)), this, SLOT(backupJobStarted(QUuid)));
    queueJob(backupClient);
    job->status = JobStatus::Started;
    emit backupJobUpdate(job);
}

void JobManager::getArchivesList()
{
    TarsnapCLI *listArchivesClient = new TarsnapCLI();
    QStringList args;
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    args << "--list-archives" << "-vv";
    listArchivesClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    listArchivesClient->setArguments(args);
    connect(listArchivesClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this, SLOT(getArchivesFinished(QUuid,QVariant,int,QString)));
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
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    if(!_tarsnapCacheDir.isEmpty())
        args << "--cachedir" << _tarsnapCacheDir;
    args << "--print-stats" << "-f" << archive->name;
    statsClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    statsClient->setArguments(args);
    connect(statsClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this, SLOT(getArchiveStatsFinished(QUuid,QVariant,int,QString)));
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
    if(!_tarsnapKeyFile.isEmpty())
        args << "--keyfile" << _tarsnapKeyFile;
    args << "-t" << "-f" << archive->name;
    contentsClient->setCommand(_tarsnapDir + CMD_TARSNAP);
    contentsClient->setArguments(args);
    connect(contentsClient, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this, SLOT(getArchiveContentsFinished(QUuid,QVariant,int,QString)));
    queueJob(contentsClient);
}

void JobManager::deleteArchives(QList<ArchivePtr> archives)
{
    if(archives.isEmpty())
    {
        qDebug() << "Empty QList<ArchivePtr> passed.";
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
    delArchives->setCommand(_tarsnapDir + CMD_TARSNAP);
    delArchives->setArguments(args);
    delArchives->setData(QVariant::fromValue(archives));
    connect(delArchives, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this, SLOT(deleteArchiveFinished(QUuid,QVariant,int,QString)));
    queueJob(delArchives);
}

void JobManager::backupJobFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(data);
    BackupJobPtr job = _backupJobMap[uuid];
    job->exitCode = exitCode;
    job->output = output;
//    job->reason = message;
    if(exitCode == 0)
    {
        job->status = JobStatus::Completed;
        ArchivePtr archive(new Archive);
        archive->name = job->name;
        archive->timestamp = QDateTime::currentDateTime();
        _archiveMap[archive->uuid] = archive;
        parseArchiveStats(output, true, archive);
        job->archive = archive;
        emit archivesList(_archiveMap.values());
    }
    else
    {
        job->status = JobStatus::Failed;
    }
    emit backupJobUpdate(job);
    _backupJobMap.remove(job->uuid);
}

void JobManager::backupJobStarted(QUuid uuid)
{
    BackupJobPtr job = _backupJobMap[uuid];
    job->status = JobStatus::Running;
    emit backupJobUpdate(job);
}

void JobManager::registerMachineFinished(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(uuid); Q_UNUSED(data)
    if(exitCode == 0)
        emit registerMachineStatus(JobStatus::Completed, output);
    else
        emit registerMachineStatus(JobStatus::Failed, output);
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
        qDebug() << "uuid not found in _archiveMap.";
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
        qDebug() << "uuid not found in _archiveMap.";
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

void JobManager::queueJob(TarsnapCLI *cli)
{
    if(cli == NULL)
    {
        qDebug() << "NULL argument";
        return;
    }
    connect(cli, SIGNAL(clientFinished(QUuid,QVariant,int,QString)), this, SLOT(dequeueJob(QUuid,QVariant,int,QString)));
    _jobMap[cli->uuid()] = cli;
    cli->setAutoDelete(false);
    _threadPool->start(cli);
    if(_jobMap.count() == 1)
        emit idle(false);
}

void JobManager::dequeueJob(QUuid uuid, QVariant data, int exitCode, QString output)
{
    Q_UNUSED(exitCode); Q_UNUSED(output); Q_UNUSED(data)
    _jobMap.remove(uuid);
    if(_jobMap.count() == 0)
        emit idle(true);
}

void JobManager::parseArchiveStats(QString tarsnapOutput, bool newArchiveOutput, ArchivePtr archive)
{
    QStringList lines = tarsnapOutput.trimmed().split('\n', QString::SkipEmptyParts);
    if(lines.count() != 5)
    {
        qDebug() << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
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
        qDebug() << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
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
        qDebug() << "Malformed output from tarsnap CLI:\n" << tarsnapOutput;
        return;
    }
    archive->notifyChanged();
}


#include "jobmanager.h"

#include <QDebug>
#include <QFileInfo>


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

void JobManager::registerMachine(QString user, QString password, QString machine, QString key)
{
    TarsnapCLI *tarClient = new TarsnapCLI();
    QStringList args;
    QFileInfo keyFile(key);
    if(keyFile.exists())
    {
        // existing key, just check with a tarsnap --print-stats command
        args << "--print-stats"<< "--keyfile"<< key;
        tarClient->setCommand(CMD_TARSNAP);
        tarClient->setArguments(args);
    }
    else
    {
        // register machine with tarsnap-keygen
        args << "--user" << user << "--machine" << machine << "--keyfile"<< key;
        tarClient->setCommand(CMD_TARSNAPKEYGEN);
        tarClient->setArguments(args);
        tarClient->setPassword(password);
        tarClient->setRequiresPassword(true);
    }
    connect(tarClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(registerMachineFinished(QUuid,int,QString)));
    _threadPool->start(tarClient);
}

void JobManager::backupNow(BackupJobPtr job)
{
    if(job.isNull())
    {
        qDebug() << "Null BackupJobPtr passed.";
        return;
    }
    _jobMap[job->uuid] = job;
    TarsnapCLI *tarClient = new TarsnapCLI(job->uuid);
    QStringList args;
    args << "-c" << "-f" << job->name;
    foreach (QUrl url, job->urls) {
        args << url.toLocalFile();
    }
    tarClient->setCommand(CMD_TARSNAP);
    tarClient->setArguments(args);
    connect(tarClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(jobFinished(QUuid,int,QString)));
    connect(tarClient, SIGNAL(clientStarted(QUuid)), this, SLOT(jobStarted(QUuid)));
    _threadPool->start(tarClient);
    job->status = JobStatus::Started;
    emit jobUpdate(job);
}

void JobManager::getArchivesList()
{
    TarsnapCLI *tarClient = new TarsnapCLI();
    QStringList args;
    args << "--list-archives" << "-vv";
    tarClient->setCommand(CMD_TARSNAP);
    tarClient->setArguments(args);
    connect(tarClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(getArchivesFinished(QUuid,int,QString)));
    _threadPool->start(tarClient);
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
    statsClient->setCommand(CMD_TARSNAP);
    statsClient->setArguments(args);
    connect(statsClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(getArchiveStatsFinished(QUuid,int,QString)));
    _threadPool->start(statsClient);
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
    contentsClient->setCommand(CMD_TARSNAP);
    contentsClient->setArguments(args);
    connect(contentsClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(getArchiveContentsFinished(QUuid,int,QString)));
    _threadPool->start(contentsClient);
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
    delArchive->setCommand(CMD_TARSNAP);
    delArchive->setArguments(args);
    connect(delArchive, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(deleteArchiveFinished(QUuid,int,QString)));
    _threadPool->start(delArchive);
}

void JobManager::jobFinished(QUuid uuid, int exitCode, QString output)
{
    BackupJobPtr job = _jobMap[uuid];
    job->exitCode = exitCode;
    job->output = output;
//    job->reason = message;
    if(exitCode == 0)
        job->status = JobStatus::Completed;
    else
        job->status = JobStatus::Failed;
    emit jobUpdate(job);
    _jobMap.remove(job->uuid);
}

void JobManager::jobStarted(QUuid uuid)
{
    BackupJobPtr job = _jobMap[uuid];
    job->status = JobStatus::Running;
    emit jobUpdate(job);
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


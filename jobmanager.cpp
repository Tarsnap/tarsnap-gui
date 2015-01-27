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
    connect(tarClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(registerClientFinished(QUuid,int,QString)));
    _threadPool->start(tarClient);
}

void JobManager::backupNow(QSharedPointer<BackupJob> job)
{
    _jobMap[job->uuid] = job;
    TarsnapCLI *tarClient = new TarsnapCLI(job->uuid);
    QStringList args;
    args << "-c" << "-f" << job->name;
    foreach (QUrl url, job->urls) {
        args << url.toLocalFile();
    }
    tarClient->setCommand(CMD_TARSNAP);
    tarClient->setArguments(args);
    connect(tarClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(jobClientFinished(QUuid,int,QString)));
    connect(tarClient, SIGNAL(clientStarted(QUuid)), this, SLOT(jobClientStarted(QUuid)));
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
    connect(tarClient, SIGNAL(clientFinished(QUuid,int,QString)), this, SLOT(listArchivesFinished(QUuid,int,QString)));
    _threadPool->start(tarClient);
}

void JobManager::jobClientFinished(QUuid uuid, int exitCode, QString output)
{
    QSharedPointer<BackupJob> job = _jobMap[uuid];
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

void JobManager::jobClientStarted(QUuid uuid)
{
    QSharedPointer<BackupJob> job = _jobMap[uuid];
    job->status = JobStatus::Running;
    emit jobUpdate(job);
}

void JobManager::registerClientFinished(QUuid uuid, int exitCode, QString output)
{
    Q_UNUSED(uuid)
    if(exitCode == 0)
        emit registerMachineStatus(JobStatus::Completed, output);
    else
        emit registerMachineStatus(JobStatus::Failed, output);
}

void JobManager::listArchivesFinished(QUuid uuid, int exitCode, QString output)
{
    Q_UNUSED(uuid)
    QList<QSharedPointer<Archive>> archives;
    if(exitCode == 0)
    {
        QStringList lines = output.trimmed().split('\n');
        foreach (QString line, lines) {
            QRegExp archiveDetailsRX("^(\\S+)\\s+(\\S+\\s+\\S+)\\s+(.+)$");
            if(-1 != archiveDetailsRX.indexIn(line))
            {
                QStringList archiveDetails = archiveDetailsRX.capturedTexts();
                archiveDetails.removeFirst();
                qDebug() << archiveDetails;
                QSharedPointer<Archive> archive(new Archive);
                archive->name = archiveDetails[0];
                archive->timestamp = QDateTime::fromString(archiveDetails[1], Qt::ISODate);
                archive->command = archiveDetails[2];
                archives.append(archive);
            }
        }
        emit archivesList(archives);
    }
}


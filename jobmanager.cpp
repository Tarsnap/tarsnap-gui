#include "jobmanager.h"

#include <QDebug>
#include <QFileInfo>


JobManager::JobManager(QObject *parent) : QObject()
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
    connect(tarClient, SIGNAL(clientFinished(int,QString,QString)), this, SLOT(registerClientFinished(int,QString,QString)));
    QMetaObject::invokeMethod(tarClient, "runClient", Qt::QueuedConnection);
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
    connect(tarClient, SIGNAL(clientFinished(int,QString,QString)), this, SLOT(jobClientFinished(int,QString,QString)));
    connect(tarClient, SIGNAL(clientStarted()), this, SLOT(jobClientStarted()));
    QMetaObject::invokeMethod(tarClient, "runClient", Qt::QueuedConnection);
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
    connect(tarClient, SIGNAL(clientFinished(int,QString,QString)), this, SLOT(listArchivesFinished(int,QString,QString)));
    QMetaObject::invokeMethod(tarClient, "runClient", Qt::QueuedConnection);
}

void JobManager::jobClientFinished(int exitCode, QString message, QString output)
{
    TarsnapCLI *tarClient =static_cast<TarsnapCLI*>(QObject::sender());
    if(tarClient == NULL) return;
    QSharedPointer<BackupJob> job = _jobMap[tarClient->uuid()];
    delete tarClient;
    job->exitCode = exitCode;
    job->output = output;
    job->reason = message;
    if(exitCode == 0)
        job->status = JobStatus::Completed;
    else
        job->status = JobStatus::Failed;
    emit jobUpdate(job);
    _jobMap.remove(job->uuid);
}

void JobManager::jobClientStarted()
{
    TarsnapCLI *tarClient =static_cast<TarsnapCLI*>(QObject::sender());
    if(tarClient == NULL) return;
    QSharedPointer<BackupJob> job = _jobMap[tarClient->uuid()];
    job->status = JobStatus::Running;
    emit jobUpdate(job);
}

void JobManager::registerClientFinished(int exitCode, QString message, QString output)
{
    Q_UNUSED(message)
    delete static_cast<TarsnapCLI*>(QObject::sender());
    if(exitCode == 0)
        emit registerMachineStatus(JobStatus::Completed, output);
    else
        emit registerMachineStatus(JobStatus::Failed, output);
}

void JobManager::listArchivesFinished(int exitCode, QString message, QString output)
{
    Q_UNUSED(message)

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


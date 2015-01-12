#include "jobmanager.h"

#include <QFileInfo>


JobManager::JobManager(QObject *parent) : QObject()
{
    // Move the operations belonging to the Job manager to a separate thread
    _managerThread.start();
    moveToThread(&_managerThread);

//    QMetaObject::invokeMethod(this, "testCall", Qt::QueuedConnection);
}

JobManager::~JobManager()
{
    _managerThread.quit();
    _managerThread.wait();
}

void JobManager::testCall()
{
    TarsnapCLI *tarClient = new TarsnapCLI();
    QStringList args;
    args << "--help";
    tarClient->setCommand(CMD_TARSNAP);
    tarClient->setArguments(args);
    QMetaObject::invokeMethod(tarClient, "runClient", Qt::QueuedConnection);
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
    connect(tarClient, SIGNAL(clientFinished(int,QString,QString)), this, SLOT(clientFinished(int,QString,QString)));
    QMetaObject::invokeMethod(tarClient, "runClient", Qt::QueuedConnection);
}

void JobManager::clientFinished(int exitStatus, QString message, QString output)
{
    delete static_cast<TarsnapCLI*>(QObject::sender());
    if(exitStatus == 0)
        emit registerMachineStatus(JobStatus::Completed, output);
    else
        emit registerMachineStatus(JobStatus::Failed, output);
}


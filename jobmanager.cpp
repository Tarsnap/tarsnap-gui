#include "jobmanager.h"

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
    args << "--user" << user << "--machine" << machine << "--keyfile"<< key;
    tarClient->setCommand(CMD_TARSNAPKEYGEN);
    tarClient->setArguments(args);
    tarClient->setPassword(password);
    tarClient->setRequiresPassword(true);
    connect(tarClient, SIGNAL(clientFinished(int,QString,QString)), this, SLOT(clientFinished(int,QString,QString)));
    QMetaObject::invokeMethod(tarClient, "runClient", Qt::QueuedConnection);
}

void JobManager::clientFinished(int exitStatus, QString message, QString output)
{
    QObject::sender()->deleteLater();
    if(exitStatus == 0)
        emit registerMachineStatus(JobStatus::Completed, output);
    else
        emit registerMachineStatus(JobStatus::Failed, output);
}


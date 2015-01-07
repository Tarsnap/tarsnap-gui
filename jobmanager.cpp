#include "jobmanager.h"

JobManager::JobManager(QObject *parent) : QObject(parent)
{
    // Move the operations belonging to the Job manager to a separate thread
    _managerThread.start();
    moveToThread(&_managerThread);
}

JobManager::~JobManager()
{
    _managerThread.quit();
    _managerThread.wait();
}


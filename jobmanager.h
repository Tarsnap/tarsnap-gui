#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include "tarsnapcli.h"

#include <QObject>
#include <QThread>

class JobManager : public QObject
{
    Q_OBJECT

    enum JobStatus { Running, Completed, Failed, Paused };

public:
    explicit JobManager(QObject *parent = 0);
    ~JobManager();

signals:
    void registerMachineStatus(JobStatus status, QString reason);

public slots:
    void testCall();
    void registerMachine(QString user, QString password, QString machine, QString key);

private slots:
    void clientFinished(int exitStatus, QString message, QString output);

private:
    QThread _managerThread; // manager runs on a separate thread
};

#endif // JOBMANAGER_H

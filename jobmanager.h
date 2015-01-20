#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include "tarsnapcli.h"

#include <QObject>
#include <QThread>
#include <QUrl>

struct BackupJob
{
    QList<QUrl> urls;
    QString     name;
};

class JobManager : public QObject
{
    Q_OBJECT

public:
    enum JobStatus { Running, Completed, Failed, Paused };

    explicit JobManager(QObject *parent = 0);
    ~JobManager();

signals:
    void registerMachineStatus(JobManager::JobStatus status, QString reason);

public slots:
    void testCall();
    void registerMachine(QString user, QString password, QString machine, QString key);
    void backupNow(BackupJob job);

private slots:
    void jobClientFinished(int exitStatus, QString message, QString output);
    void registerClientFinished(int exitStatus, QString message, QString output);


private:
    QThread _managerThread; // manager runs on a separate thread
};

#endif // JOBMANAGER_H

#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include "tarsnapcli.h"

#include <QObject>
#include <QThread>
#include <QUrl>
#include <QUuid>
#include <QMap>
#include <QSharedPointer>
#include <QDateTime>

class JobManager;

enum JobStatus { Started, Running, Completed, Failed, Paused, Unknown };

class BackupJob
{
public:
    QList<QUrl>           urls;
    QString               name;
    QUuid                 uuid;
    JobStatus             status;
    int                   exitCode;
    QString               reason;
    QString               output;

    BackupJob():uuid(QUuid::createUuid()),status(JobStatus::Unknown){}
};

class Archive
{
public:
    QString     name;
    QDateTime   timestamp;
    qint64      sizeTotal;
    qint64      sizeCompressed;
    qint64      sizeUniqueTotal;
    qint64      sizeUniqueCompressed;
    QString     command;
    QStringList contents;

    Archive():sizeTotal(0),sizeCompressed(0),sizeUniqueTotal(0),sizeUniqueCompressed(0){}
};

class JobManager : public QObject
{
    Q_OBJECT

public:
    explicit JobManager(QObject *parent = 0);
    ~JobManager();

signals:
    void registerMachineStatus(JobStatus status, QString reason);
    void jobUpdate(QSharedPointer<BackupJob> job);
    void archivesList(QList<QSharedPointer<Archive>> archives);

public slots:
    void registerMachine(QString user, QString password, QString machine, QString key);
    void backupNow(QSharedPointer<BackupJob> job);
    void getArchivesList();

private slots:
    void jobClientFinished(int exitCode, QString message, QString output);
    void jobClientStarted();
    void registerClientFinished(int exitCode, QString message, QString output);
    void listArchivesFinished(int exitCode, QString message, QString output);

private:
    QThread                   _managerThread; // manager runs on a separate thread
    QMap<QUuid, QSharedPointer<BackupJob>>    _jobMap;
};

#endif // JOBMANAGER_H

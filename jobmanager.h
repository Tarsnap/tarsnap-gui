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
#include <QThreadPool>


#define CMD_TARSNAP         "tarsnap"
#define CMD_TARSNAPKEYGEN   "tarsnap-keygen"

class Archive: public QObject
{
    Q_OBJECT

public:
    QUuid       uuid;
    QString     name;
    QDateTime   timestamp;
    qint64      sizeTotal;
    qint64      sizeCompressed;
    qint64      sizeUniqueTotal;
    qint64      sizeUniqueCompressed;
    QString     command;
    QStringList contents;

    Archive():uuid(QUuid::createUuid()),sizeTotal(0),sizeCompressed(0),sizeUniqueTotal(0),sizeUniqueCompressed(0){}

    void notifyChanged() { emit changed(); }
    QString archiveStats() {
        QString stats;
        if(sizeTotal == 0 || sizeUniqueCompressed == 0)
            return stats;
        stats.append(tr("\t\tTotal size\tCompressed size\n"
                     "this archive\t%1\t%2\n"
                     "unique data\t%3\t%4").arg(sizeTotal).arg(sizeCompressed)
                     .arg(sizeUniqueTotal).arg(sizeUniqueCompressed));
        return stats;
    }

signals:
    void changed();
};

typedef QSharedPointer<Archive> ArchivePtr;

Q_DECLARE_METATYPE(ArchivePtr)

enum JobStatus { Started, Running, Completed, Failed, Paused, Unknown };

class BackupJob
{
public:
    QUuid                 uuid;
    QList<QUrl>           urls;
    QString               name;
    JobStatus             status;
    int                   exitCode;
    QString               output;
    ArchivePtr            archive;

    BackupJob():uuid(QUuid::createUuid()),status(JobStatus::Unknown){}
};

typedef QSharedPointer<BackupJob> BackupJobPtr;

class JobManager : public QObject
{
    Q_OBJECT

public:
    explicit JobManager(QObject *parent = 0);
    ~JobManager();

signals:
    void idle(bool status); // signal if we are working on jobs or not
    void registerMachineStatus(JobStatus status, QString reason);
    void fsckStatus(JobStatus status, QString reason);
    void backupJobUpdate(BackupJobPtr job);
    void archivesList(QList<ArchivePtr> archives);
    void archivesDeleted(QList<ArchivePtr> archives);
    void overallStats(qint64 sizeTotal, qint64 sizeCompressed, qint64 sizeUniqueTotal
                      , qint64 sizeUniqueCompressed, qint64 archiveCount, qreal credit
                      , QString accountStatus);

public slots:
    void reloadSettings();

    void registerMachine(QString user, QString password, QString machine
                         ,QString key, QString tarsnapPath, QString cachePath);
    void backupNow(BackupJobPtr job);
    void getArchivesList();
    void getArchiveStats(ArchivePtr archive);
    void getArchiveContents(ArchivePtr archive);
    void deleteArchives(QList<ArchivePtr> archives);
    void getOverallStats();
    void runFsck();

private slots:
    void backupJobFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void backupJobStarted(QUuid uuid);
    void registerMachineFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void getArchivesFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void getArchiveStatsFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void getArchiveContentsFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void deleteArchiveFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void overallStatsFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void fsckFinished(QUuid uuid, QVariant data, int exitCode, QString output);

    void queueJob(TarsnapCLI *cli);
    void dequeueJob(QUuid uuid, QVariant data, int exitCode, QString output);
    void parseArchiveStats(QString tarsnapOutput, bool newArchiveOutput, ArchivePtr archive);

private:
    QString makeTarsnapCommand(QString cmd);

private:
    QString                      _tarsnapDir;
    QString                      _tarsnapCacheDir;
    QString                      _tarsnapKeyFile;
    QThread                      _managerThread; // manager runs on a separate thread
    QMap<QUuid, BackupJobPtr>    _backupJobMap;
    QMap<QUuid, ArchivePtr>      _archiveMap;
    QMap<QUuid, TarsnapCLI*>     _jobMap;
    QThreadPool                 *_threadPool;
    bool                         _aggressiveNetworking;
};

#endif // JOBMANAGER_H

#include "backenddata.h"

WARNINGS_DISABLE
#include <QChar>
#include <QDateTime>
#include <QLatin1String>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
WARNINGS_ENABLE

#include "debug.h"

#include "backuptask.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "persistentmodel/persistentstore.h"
#include "tasks/tasks-tarsnap.h"

BackendData::BackendData()
{
}

QMap<QString, JobPtr> BackendData::jobs()
{
    return _jobMap;
}

QMap<QString, ArchivePtr> BackendData::archives()
{
    return _archiveMap;
}

quint64 BackendData::numArchives()
{
    return static_cast<quint64>(_archiveMap.count());
}

bool BackendData::loadArchives()
{
    _archiveMap.clear();

    // Get data from the store.
    if(!global_store->initialized())
    {
        DEBUG << "PersistentStore was not initialized properly.";
        return false;
    }
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(QLatin1String("select name from archives")))
    {
        DEBUG << query.lastError().text();
        return false;
    }
    if(!global_store->runQuery(query))
    {
        DEBUG << "loadArchives query failed.";
        return false;
    }

    // Process data from the store.
    const int index = query.record().indexOf("name");
    while(query.next())
    {
        ArchivePtr archive(new Archive);
        archive->setName(query.value(index).toString());
        archive->load();
        _archiveMap[archive->name()] = archive;
    }
    return true;
}

QList<ArchivePtr> BackendData::findMatchingArchives(const QString &jobPrefix)
{
    const QString prefix = jobPrefix + QChar('_');

    // Get all archives beginning with the relevant prefix who do
    // not already belong to a job.
    QList<ArchivePtr> matching;
    for(const ArchivePtr &archive : _archiveMap)
    {
        if(archive->name().startsWith(prefix) && archive->jobRef().isEmpty())
            matching << archive;
    }
    return matching;
}

ArchivePtr BackendData::newArchive(BackupTaskDataPtr backupTaskData,
                                   bool              truncated)
{
    ArchivePtr archive(new Archive);
    archive->setName(backupTaskData->name());
    archive->setCommand(backupTaskData->command());
    archive->setJobRef(backupTaskData->jobRef());

    // Was the archive creation interrupted?
    if(truncated)
    {
        archive->setName(archive->name().append(".part"));
        archive->setTruncated(true);
    }

    // Lose milliseconds precision by converting to Unix timestamp and back.
    // So that a subsequent comparison in getArchiveListFinished won't fail.
    archive->setTimestamp(
        QDateTime::fromTime_t(backupTaskData->timestamp().toTime_t()));

    // Link it.
    backupTaskData->setArchive(archive);

    // Bail if it's a --dry-run.
    if(backupTaskData->optionDryRun())
        return archive;

    // Save data and add to the map.
    archive->save();
    _archiveMap.insert(archive->name(), archive);

    // Ensure that the archive is attached to the job (if applicable).
    if(!archive->jobRef().isEmpty())
    {
        for(const JobPtr &job : _jobMap)
        {
            if(job->objectKey() == archive->jobRef())
                emit job->loadArchives();
        }
    }
    return archive;
}

QList<ArchivePtr>
BackendData::setArchivesFromList(QList<struct archive_list_data> metadatas)
{
    QList<ArchivePtr> newArchives;

    QMap<QString, ArchivePtr> nextArchiveMap;
    for(const struct archive_list_data &metadata : metadatas)
    {
        ArchivePtr archive =
            _archiveMap.value(metadata.archiveName, ArchivePtr(new Archive));
        if(!archive->objectKey().isEmpty()
           && (archive->timestamp() != metadata.timestamp))
        {
            // There is a different archive with the same name on the remote
            archive->purge();
            archive.clear();
            archive = archive.create();
        }
        if(archive->objectKey().isEmpty())
        {
            // New archive
            archive->setName(metadata.archiveName);
            archive->setTimestamp(metadata.timestamp);
            archive->setCommand(metadata.command);
            // Automagically set Job ownership
            for(const JobPtr &job : _jobMap)
            {
                if(archive->name().startsWith(job->archivePrefix()))
                    archive->setJobRef(job->objectKey());
            }
            archive->save();
            newArchives.append(archive);
        }
        nextArchiveMap.insert(archive->name(), archive);
        _archiveMap.remove(archive->name());
    }
    // Purge archives left in old _archiveMap (not mirrored by the remote)
    for(const ArchivePtr &archive : _archiveMap)
    {
        archive->purge();
    }
    _archiveMap.clear();
    _archiveMap = nextArchiveMap;
    for(const JobPtr &job : _jobMap)
    {
        emit job->loadArchives();
    }
    return newArchives;
}

void BackendData::removeArchives(QList<ArchivePtr> archives)
{
    for(const ArchivePtr &archive : archives)
    {
        _archiveMap.remove(archive->name());
        archive->purge();
    }
}

bool BackendData::loadJobs()
{
    _jobMap.clear();

    // Get data from the store.
    if(!global_store->initialized())
    {
        DEBUG << "PersistentStore was not initialized properly.";
        return false;
    }
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(QLatin1String("select name from jobs")))
    {
        DEBUG << query.lastError().text();
        return false;
    }
    if(!global_store->runQuery(query))
    {
        DEBUG << "loadJobs query failed.";
        return false;
    }

    // Process data from the store.
    const int index = query.record().indexOf("name");
    while(query.next())
    {
        JobPtr job(new Job);
        job->setName(query.value(index).toString());
        connect(job.data(), &Job::loadArchives, this,
                &BackendData::loadJobArchives);
        job->load();
        _jobMap[job->name()] = job;
    }
    return true;
}

void BackendData::deleteJob(JobPtr job)
{
    // Clear JobRef for assigned Archives.
    for(const ArchivePtr &archive : job->archives())
    {
        archive->setJobRef("");
        archive->save();
    }

    job->purge();
    _jobMap.remove(job->name());
}

void BackendData::loadJobArchives()
{
    Job              *job = qobject_cast<Job *>(sender());
    QList<ArchivePtr> archives;
    for(const ArchivePtr &archive : _archiveMap)
    {
        if(archive->jobRef() == job->objectKey())
            archives << archive;
    }
    job->setArchives(archives);
}

void BackendData::addJob(JobPtr job)
{
    _jobMap[job->name()] = job;
    connect(job.data(), &Job::loadArchives, this,
            &BackendData::loadJobArchives);
}

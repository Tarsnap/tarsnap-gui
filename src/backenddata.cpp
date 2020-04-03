#include "backenddata.h"

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
    return true;
}

bool BackendData::loadJobs()
{
    return true;
}

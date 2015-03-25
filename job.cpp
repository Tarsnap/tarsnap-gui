#include "job.h"

Job::Job(QObject *parent) : QObject(parent)
{

}

Job::~Job()
{

}
QString Job::name() const
{
    return _name;
}

void Job::setName(const QString &name)
{
    _name = name;
}
QList<QUrl> Job::urls() const
{
    return _urls;
}

void Job::setUrls(const QList<QUrl> &urls)
{
    _urls = urls;
}
QList<ArchivePtr> Job::archives() const
{
    return _archives;
}

void Job::setArchives(const QList<ArchivePtr> &archives)
{
    _archives = archives;
}
QUuid Job::uuid() const
{
    return _uuid;
}

void Job::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}






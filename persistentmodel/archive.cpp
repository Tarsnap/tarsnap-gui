#include "archive.h"

Archive::Archive(QObject *parent) : QObject(parent), _uuid(QUuid::createUuid()), _sizeTotal(0),
    _sizeCompressed(0), _sizeUniqueTotal(0), _sizeUniqueCompressed(0)
{

}

Archive::~Archive()
{

}

void Archive::save()
{

}

void Archive::load()
{

}

void Archive::purge()
{

}

QString Archive::archiveStats()
{
    QString stats;
    if(_sizeTotal == 0 || _sizeUniqueCompressed == 0)
        return stats;
    stats.append(tr("\t\tTotal size\tCompressed size\n"
                    "this archive\t%1\t\t%2\n"
                    "unique data\t%3\t\t%4").arg(_sizeTotal).arg(_sizeCompressed)
                 .arg(_sizeUniqueTotal).arg(_sizeUniqueCompressed));
    return stats;
}
QStringList Archive::contents() const
{
    return _contents;
}

void Archive::setContents(const QStringList &value)
{
    _contents = value;
}

QString Archive::command() const
{
    return _command;
}

void Archive::setCommand(const QString &value)
{
    _command = value;
}

qint64 Archive::sizeUniqueCompressed() const
{
    return _sizeUniqueCompressed;
}

void Archive::setSizeUniqueCompressed(const qint64 &value)
{
    _sizeUniqueCompressed = value;
}

qint64 Archive::sizeUniqueTotal() const
{
    return _sizeUniqueTotal;
}

void Archive::setSizeUniqueTotal(const qint64 &value)
{
    _sizeUniqueTotal = value;
}

qint64 Archive::sizeCompressed() const
{
    return _sizeCompressed;
}

void Archive::setSizeCompressed(const qint64 &value)
{
    _sizeCompressed = value;
}

qint64 Archive::sizeTotal() const
{
    return _sizeTotal;
}

void Archive::setSizeTotal(const qint64 &value)
{
    _sizeTotal = value;
}

QDateTime Archive::timestamp() const
{
    return _timestamp;
}

void Archive::setTimestamp(const QDateTime &value)
{
    _timestamp = value;
}

QString Archive::name() const
{
    return _name;
}

void Archive::setName(const QString &value)
{
    _name = value;
}


QUuid Archive::uuid() const
{
    return _uuid;
}

void Archive::setUuid(const QUuid &value)
{
    _uuid = value;
}

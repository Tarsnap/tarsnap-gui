#include "persistentmodel/archive.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QLatin1String>
#include <QObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include "debug.h"

#include "persistentmodel/persistentstore.h"

Archive::Archive(QObject *parent)
    : PersistentObject(parent),
      _truncated(false),
      _truncatedInfo(""),
      _sizeTotal(0),
      _sizeCompressed(0),
      _sizeUniqueTotal(0),
      _sizeUniqueCompressed(0),
      _deleteScheduled(false)
{
}

void Archive::save()
{
    bool    exists = doesKeyExist(_name);
    QString queryString;
    // Prepare query: either updating or creating an entry.
    if(exists)
    {
        queryString =
            QLatin1String("update archives set name=?, timestamp=?,"
                          " truncated=?, truncatedInfo=?, sizeTotal=?,"
                          " sizeCompressed=?, sizeUniqueTotal=?,"
                          " sizeUniqueCompressed=?, command=?, contents=?,"
                          " jobRef=?"
                          " where name=?");
    }
    else
    {
        queryString = QLatin1String(
            "insert into archives(name, timestamp, truncated, truncatedInfo,"
            " sizeTotal, sizeCompressed, sizeUniqueTotal,"
            " sizeUniqueCompressed, command, contents, jobRef)"
            " values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    }
    // Get database instance and create query object.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(queryString))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Fill in missing values in query string.
    query.addBindValue(_name);
    query.addBindValue(_timestamp.toTime_t());
    query.addBindValue(_truncated);
    query.addBindValue(_truncatedInfo);
    query.addBindValue(_sizeTotal);
    query.addBindValue(_sizeCompressed);
    query.addBindValue(_sizeUniqueTotal);
    query.addBindValue(_sizeUniqueCompressed);
    query.addBindValue(_command);
    query.addBindValue(_contents);
    query.addBindValue(_jobRef);
    if(exists)
        query.addBindValue(_name);
    // Run query.
    if(!global_store->runQuery(query))
        DEBUG << "Failed to save Archive entry.";
    setObjectKey(_name);
    emit changed();
}

void Archive::load()
{
    // Sanity check.
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to load Archive object with empty _name key.";
        return;
    }
    // Get database instance and prepare query.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(QLatin1String("select * from archives where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Fill in missing value in query string.
    query.addBindValue(_name);
    // Run query and extract information.
    if(global_store->runQuery(query) && query.next())
    {
        _timestamp = QDateTime::fromTime_t(
            query.value(query.record().indexOf("timestamp")).toUInt());
        _truncated = query.value(query.record().indexOf("truncated")).toBool();
        _truncatedInfo =
            query.value(query.record().indexOf("truncatedInfo")).toString();
        _sizeTotal =
            query.value(query.record().indexOf("sizeTotal")).toULongLong();
        _sizeCompressed =
            query.value(query.record().indexOf("sizeCompressed")).toULongLong();
        _sizeUniqueTotal =
            query.value(query.record().indexOf("sizeUniqueTotal"))
                .toULongLong();
        _sizeUniqueCompressed =
            query.value(query.record().indexOf("sizeUniqueCompressed"))
                .toULongLong();
        _command = query.value(query.record().indexOf("command")).toString();
        _contents =
            query.value(query.record().indexOf("contents")).toByteArray();
        _jobRef = query.value(query.record().indexOf("jobRef")).toString();
        setObjectKey(_name);
    }
    else
    {
        DEBUG << "Archive object with key " << _name << " not found.";
    }
}

void Archive::purge()
{
    // Sanity checks.
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to delete Archive object with empty _name key.";
        return;
    }
    if(!doesKeyExist(_name))
    {
        DEBUG << "No Archive object with key " << _name;
        return;
    }
    // Get database instance and prepare query.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(QLatin1String("delete from archives where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Fill in missing value in query string.
    query.addBindValue(_name);
    // Run query.
    if(!global_store->runQuery(query))
        DEBUG << "Failed to remove Archive entry.";
    setObjectKey("");
    emit purged();
}

bool Archive::doesKeyExist(const QString &key)
{
    // Sanity check.
    if(key.isEmpty())
    {
        DEBUG << "doesKeyExist method called with empty args";
        return false;
    }
    // Get database instance and prepare query.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(
           QLatin1String("select name from archives where name = ?")))
    {
        DEBUG << query.lastError().text();
        return false;
    }
    // Fill in missing value in query string.
    query.addBindValue(key);
    // Run query.
    if(global_store->runQuery(query))
    {
        if(query.next())
            return true;
    }
    else
    {
        DEBUG << "Failed to run doesKeyExist query for an Archive.";
    }
    return false;
}

QString Archive::archiveStats()
{
    QString stats;
    if((_sizeTotal == 0) || (_sizeUniqueCompressed == 0))
        return stats;
    stats.append(tr("\t\tTotal size\tCompressed size\n"
                    "this archive\t%1\t\t%2\n"
                    "unique data\t%3\t\t%4")
                     .arg(_sizeTotal)
                     .arg(_sizeCompressed)
                     .arg(_sizeUniqueTotal)
                     .arg(_sizeUniqueCompressed));
    return stats;
}

bool Archive::deleteScheduled() const
{
    return _deleteScheduled;
}

void Archive::setDeleteScheduled(bool deleteScheduled)
{
    _deleteScheduled = deleteScheduled;
    emit changed();
}

bool Archive::truncated() const
{
    return _truncated;
}

void Archive::setTruncated(bool truncated)
{
    _truncated = truncated;
}

QString Archive::truncatedInfo() const
{
    return _truncatedInfo;
}

void Archive::setTruncatedInfo(const QString &truncatedInfo)
{
    _truncatedInfo = truncatedInfo;
}

QString Archive::jobRef() const
{
    return _jobRef;
}

void Archive::setJobRef(const QString &jobRef)
{
    _jobRef = jobRef;
}

bool Archive::hasPreservePaths()
{
    return _command.contains(" -P ", Qt::CaseSensitive);
}

QString Archive::contents() const
{
    if(!_contents.isEmpty())
        return qUncompress(_contents);
    else
        return QString();
}

void Archive::setContents(const QString &value)
{
    _contents = qCompress(value.toLatin1());
}

QString Archive::command() const
{
    return _command;
}

void Archive::setCommand(const QString &value)
{
    _command = value;
}

quint64 Archive::sizeUniqueCompressed() const
{
    return _sizeUniqueCompressed;
}

void Archive::setSizeUniqueCompressed(const quint64 &value)
{
    _sizeUniqueCompressed = value;
}

quint64 Archive::sizeUniqueTotal() const
{
    return _sizeUniqueTotal;
}

void Archive::setSizeUniqueTotal(const quint64 &value)
{
    _sizeUniqueTotal = value;
}

quint64 Archive::sizeCompressed() const
{
    return _sizeCompressed;
}

void Archive::setSizeCompressed(const quint64 &value)
{
    _sizeCompressed = value;
}

quint64 Archive::sizeTotal() const
{
    return _sizeTotal;
}

void Archive::setSizeTotal(const quint64 &value)
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

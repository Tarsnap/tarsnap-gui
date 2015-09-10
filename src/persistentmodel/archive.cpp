#include "archive.h"
#include "debug.h"

Archive::Archive(QObject *parent) : QObject(parent), _sizeTotal(0),
    _sizeCompressed(0), _sizeUniqueTotal(0), _sizeUniqueCompressed(0)
{
}

Archive::~Archive()
{
}

void Archive::save()
{
    bool exists = findObjectWithKey(_name);
    QString queryString;
    if(exists)
        queryString = QLatin1String("update archives set name=?, timestamp=?, sizeTotal=?, sizeCompressed=?,"
                                    " sizeUniqueTotal=?, sizeUniqueCompressed=?, command=?, contents=?, jobRef=?"
                                    " where name=?");
    else
        queryString = QLatin1String("insert into archives(name, timestamp, sizeTotal, sizeCompressed,"
                                    " sizeUniqueTotal, sizeUniqueCompressed, command, contents, jobRef)"
                                    " values(?, ?, ?, ?, ?, ?, ?, ?, ?)");
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(queryString))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(_name);
    query.addBindValue(_timestamp.toTime_t());
    query.addBindValue(_sizeTotal);
    query.addBindValue(_sizeCompressed);
    query.addBindValue(_sizeUniqueTotal);
    query.addBindValue(_sizeUniqueCompressed);
    query.addBindValue(_command);
    query.addBindValue(_contents.join('\n'));
    query.addBindValue(_jobRef);
    if(exists)
        query.addBindValue(_name);

    store.runQuery(query);
    setObjectKey(_name);
    emit changed();
}

void Archive::load()
{
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to load Archive object with empty _name key.";
        return;
    }
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select * from archives where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(_name);
    if(store.runQuery(query) && query.next())
    {
        _timestamp = QDateTime::fromTime_t(query.value(query.record().indexOf("timestamp")).toUInt());
        _sizeTotal = query.value(query.record().indexOf("sizeTotal")).toUInt();
        _sizeCompressed = query.value(query.record().indexOf("sizeCompressed")).toUInt();
        _sizeUniqueTotal = query.value(query.record().indexOf("sizeUniqueTotal")).toUInt();
        _sizeUniqueCompressed = query.value(query.record().indexOf("sizeUniqueCompressed")).toUInt();
        _command = query.value(query.record().indexOf("command")).toString();
        _contents = query.value(query.record().indexOf("contents")).toString().split('\n', QString::SkipEmptyParts);
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
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to delete Archive object with empty _name key.";
        return;
    }
    if(!findObjectWithKey(_name))
    {
        DEBUG << "No Archive object with key " << _name;
        return;
    }
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("delete from archives where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(_name);
    store.runQuery(query);
    setObjectKey("");
}

bool Archive::findObjectWithKey(QString key)
{
    bool found = false;
    if(key.isEmpty())
    {
        DEBUG << "findObjectWithKey method called with empty args";
        return found;
    }
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select name from archives where name = ?")))
    {
        DEBUG << query.lastError().text();
        return found;
    }
    query.addBindValue(key);
    if(store.runQuery(query) && query.next())
    {
        found = true;
    }
    return found;
}

QString Archive::archiveStats()
{
    QString stats;
    if((_sizeTotal == 0) || (_sizeUniqueCompressed == 0))
        return stats;
    stats.append(tr("\t\tTotal size\tCompressed size\n"
                    "this archive\t%1\t\t%2\n"
                    "unique data\t%3\t\t%4").arg(_sizeTotal).arg(_sizeCompressed).arg(_sizeUniqueTotal)
                 .arg(_sizeUniqueCompressed));
    return stats;
}
QString Archive::jobRef() const
{
    return _jobRef;
}

void Archive::setJobRef(const QString &jobRef)
{
    _jobRef = jobRef;
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


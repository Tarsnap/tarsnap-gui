#include "journal.h"

#include "debug.h"

Journal::Journal(QObject *parent) : QObject(parent)
{
}

Journal::~Journal()
{
    log("==Session end==");
}

void Journal::load()
{
    _log.clear();
    PersistentStore &store = getStore();
    QSqlQuery        query = store.createQuery();
    if(!query.prepare(QLatin1String("select * from journal")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    if(store.runQuery(query))
    {
        while(query.next())
        {
            QDateTime timestamp = QDateTime::fromTime_t(
                query.value(query.record().indexOf("timestamp")).toUInt());
            QString log = query.value(query.record().indexOf("log")).toString();
            _log.push_back(LogEntry{timestamp, log});
        }
        log("==Session start==");
    }
    else
    {
        DEBUG << "Failed loading the Journal from the Persistent Store.";
    }
}

void Journal::purge()
{
    PersistentStore &store = getStore();
    QSqlQuery        query = store.createQuery();
    if(!query.prepare(QLatin1String("delete from journal")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    if(store.runQuery(query))
    {
        _log.clear();
        emit journal(_log);
    }
    else
    {
        DEBUG << "Failed to purge the Journal from the Persistent Store.";
    }
}

void Journal::log(QString message)
{
    LogEntry log{QDateTime::currentDateTime(), message};
    _log.push_back(log);

    PersistentStore &store = getStore();
    QSqlQuery        query = store.createQuery();
    if(!query.prepare(QLatin1String("insert into journal(timestamp, log) "
                                    "values(?, ?)")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(log.timestamp.toTime_t());
    query.addBindValue(log.message.remove(QRegExp("<[^>]*>")));
    if(!store.runQuery(query))
        DEBUG << "Failed to add Journal entry.";

    emit logEntry(log);
}

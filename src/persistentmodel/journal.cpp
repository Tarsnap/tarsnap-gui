#include "journal.h"

#include "debug.h"

Journal::Journal(QObject *parent) : QObject(parent)
{
}

Journal::~Journal()
{
    log("Session end");
}

void Journal::load()
{
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
            QDateTime timestamp = QDateTime::fromTime_t(query.value(query.record().indexOf("timestamp")).toULongLong());
            QString log = query.value(query.record().indexOf("log")).toString();
            _log.insertMulti(timestamp, log);
        }
        log("Session start");
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
        _log.clear();
    else
        DEBUG << "Failed to purge the Journal from the Persistent Store.";
}

void Journal::log(QString message)
{
    QDateTime timestamp(QDateTime::currentDateTime());
    _log.insertMulti(timestamp, message);

    PersistentStore &store = getStore();
    QSqlQuery        query = store.createQuery();
    if(!query.prepare(QLatin1String("insert into journal(timestamp, log) "
                                    "values(?, ?)")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(timestamp.toTime_t());
    query.addBindValue(message);
    if(!store.runQuery(query))
        DEBUG << "Failed to add Journal entry.";

    emit logEntry(timestamp, message);
}

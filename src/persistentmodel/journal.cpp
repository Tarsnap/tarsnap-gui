#include "journal.h"

WARNINGS_DISABLE
#include <QtGlobal>
WARNINGS_ENABLE

#include "debug.h"

static qint64 dateToEpoch(const QDateTime date)
{
#if(QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    return date.toSecsSinceEpoch();
#else
    return date.toMSecsSinceEpoch() / 1000;
#endif
}

static QDateTime epochToDate(const qint64 secs)
{
#if(QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    return QDateTime::fromSecsSinceEpoch(secs);
#else
    return QDateTime::fromMSecsSinceEpoch(secs * 1000);
#endif
}

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
    // Get database instance and prepare query.
    PersistentStore &store = getStore();
    QSqlQuery        query = store.createQuery();
    if(!query.prepare(QLatin1String("select * from journal")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Load stored log entries from the database.
    if(store.runQuery(query))
    {
        while(query.next())
        {
            // Parse stored date and time.
            QDateTime timestamp =
                epochToDate(query.value(query.record().indexOf("timestamp"))
                                .value<qint64>());
            // Extract the log message.
            QString log = query.value(query.record().indexOf("log")).toString();
            // Creates a LogEntry and appends it to _log.
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
    // Get database instance and prepare query.
    PersistentStore &store = getStore();
    QSqlQuery        query = store.createQuery();
    if(!query.prepare(QLatin1String("delete from journal")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Run "delete" query.
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
    // Create a LogEntry with the current timestamp, after
    // stripping HTML commands from the log message.
    LogEntry log{QDateTime::currentDateTime(),
                 message.remove(QRegExp("<[^>]*>"))};
    _log.push_back(log);
    emit logEntry(log);

    // Get database instance and prepare query.
    PersistentStore &store = getStore();
    QSqlQuery        query = store.createQuery();
    if(!query.prepare(QLatin1String("insert into journal(timestamp, log) "
                                    "values(?, ?)")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Fill in missing values in query string.
    query.addBindValue(dateToEpoch(log.timestamp));
    query.addBindValue(log.message);
    // Run query.
    if(!store.runQuery(query))
        DEBUG << "Failed to add Journal entry.";
}

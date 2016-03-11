#include "persistentstore.h"
#include "debug.h"

#include <QFile>
#include <QFileInfo>
#include <QSettings>

PersistentStore::PersistentStore(QObject *parent)
    : QObject(parent), _initialized(false)
{
    init();
}

bool PersistentStore::init()
{
    QMutexLocker locker(&_mutex);

    QSettings settings;
    QString   appdata = settings.value("app/app_data").toString();
    if(appdata.isEmpty())
    {
        DEBUG << "Error creating the PersistentStore: app.appdata not set.";
        return false;
    }
    bool      create = false;
    QString   dbUrl(appdata + QDir::separator() + DEFAULT_DBNAME);
    QFileInfo dbFileInfo(dbUrl);

    _db = _db.addDatabase("QSQLITE", "tarsnap");
    _db.setConnectOptions("QSQLITE_OPEN_URI");
    _db.setDatabaseName(dbUrl);

    if(!dbFileInfo.exists())
    {
        create = true;
    }
    else if(!dbFileInfo.isFile() || !dbFileInfo.isReadable())
    {
        DEBUG
            << "Error creating the PersistentStore: DB file is not accessible "
            << dbUrl;
        return false;
    }
    else if(!_db.open())
    {
        DEBUG << "Error opening the PersistentStore DB: "
              << _db.lastError().text();
        return false;
    }
    else
    {
        QStringList tables = _db.tables();
        if(!tables.contains("archives", Qt::CaseInsensitive))
        {
            _db.close();
            DEBUG << "Invalid PersistentStore DB found. Attempting to recover.";
            QString newName(dbUrl + "." +
                            QString::number(QDateTime::currentMSecsSinceEpoch()));
            if(!QFile::rename(dbUrl, newName))
            {
                DEBUG << "Failed to rename current invalid PersistentStore DB. "
                         "Please manually cleanup the DB directory "
                      << appdata;
                return false;
            }
            create = true;
        }
        else
        {
            if(!tables.contains("version", Qt::CaseInsensitive))
            {
                if(!upgradeVersion0())
                {
                    DEBUG << "Failed to upgrade PersistentStore DB. It's best "
                             "to start from scratch by purging the existing DB "
                             "in "
                          << appdata;
                    return false;
                }
            }
            int       version = -1;
            QSqlQuery query(_db);
            if(query.exec("SELECT version FROM version"))
            {
                query.next();
                version = query.value(0).toInt();
            }
            else
            {
                DEBUG << "Failed to get current DB version: "
                      << query.lastError().text();
                return false;
            }
            if((version == 0) && upgradeVersion1())
            {
                DEBUG << "DB upgraded to version 1.";
                version = 1;
            }
            if((version == 1) && upgradeVersion2())
            {
                DEBUG << "DB upgraded to version 2.";
                version = 2;
            }
            if((version == 2) && upgradeVersion3())
            {
                DEBUG << "DB upgraded to version 3.";
                version = 3;
            }
        }
    }

    if(create)
    {
        QFile dbTemplate(":/dbtemplate.db");
        if(!dbTemplate.copy(dbUrl))
        {
            DEBUG << "Failed to create the PersistentStore DB.";
            return false;
        }
        // Work around the fact that QFile::copy from the resource system does
        // not set u+w on the resulted file
        QFile dbFile(dbUrl);
        dbFile.setPermissions(dbFile.permissions() | QFileDevice::WriteOwner);
        dbFile.close();
        if(!_db.open())
        {
            DEBUG << "Error opening the PersistentStore DB: "
                  << _db.lastError().text();
            return false;
        }
    }
    return _initialized = true;
}

void PersistentStore::deinit()
{
    QMutexLocker locker(&_mutex);

    if(_initialized)
    {
        _db.close();
        _db = QSqlDatabase();
        _db.removeDatabase("tarsnap");
        _initialized = false;
    }
}

QSqlQuery PersistentStore::createQuery()
{
    if(_initialized)
    {
        return QSqlQuery(_db);
    }
    else
    {
        DEBUG << "PersistentStore not initialized.";
        return QSqlQuery();
    }
}

PersistentStore::~PersistentStore()
{
    deinit();
}

void PersistentStore::purge()
{
    if(_initialized)
    {
        QString dbUrl = _db.databaseName();
        deinit();
        QFile dbFile(dbUrl);
        if(dbFile.exists())
            dbFile.remove();
        else
            DEBUG << "DB file not accessible: " << dbUrl;
    }
    else
    {
        DEBUG << "DB not initialized.";
    }
}

void PersistentStore::lock()
{
    _mutex.lock();
}

void PersistentStore::unlock()
{
    _mutex.unlock();
}

bool PersistentStore::runQuery(QSqlQuery query)
{
    QMutexLocker locker(&_mutex);

    bool result = false;
    if(_initialized)
    {
        if(!query.exec())
            DEBUG << query.lastError().text();
        else
            result = true;
    }
    else
    {
        DEBUG << "DB not initialized.";
    }

    return result;
}

bool PersistentStore::upgradeVersion0()
{
    bool      result = false;
    QSqlQuery query(_db);

    if((result =
            query.exec("CREATE TABLE version (version INTEGER NOT NULL);")))
        result = query.exec("INSERT INTO version VALUES (0);");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 0." << _db;
    }
    return result;
}

bool PersistentStore::upgradeVersion1()
{
    bool      result = false;
    QSqlQuery query(_db);

    if((result = query.exec(
            "ALTER TABLE jobs ADD COLUMN optionScheduledEnabled INTEGER;")))
        result = query.exec("UPDATE version SET version = 1;");

    // Handle the special case, since I started versioning DB after two app
    // releases and this change in between...
    if(!result && query.lastError().text().contains("duplicate column name"))
        result = query.exec("UPDATE version SET version = 1;");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 1." << _db.databaseName();
    }
    return result;
}

bool PersistentStore::upgradeVersion2()
{
    bool      result = false;
    QSqlQuery query(_db);

    if((result =
            query.exec("ALTER TABLE jobs ADD COLUMN optionSkipFiles INTEGER;")))
        if((result = query.exec(
                "ALTER TABLE jobs ADD COLUMN optionSkipFilesPatterns TEXT;")))
            result = query.exec("UPDATE version SET version = 2;");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 2." << _db.databaseName();
    }
    return result;
}

bool PersistentStore::upgradeVersion3()
{
    bool      result = false;
    QSqlQuery query(_db);

    if((result = query.exec(
            "ALTER TABLE jobs ADD COLUMN optionSkipNoDump INTEGER;")))
        if((result = query.exec("UPDATE archives SET contents=\"\";")))
            if((result = query.exec("CREATE TABLE journal (timestamp INTEGER "
                                    "NOT NULL, log TEXT);")))
                result = query.exec("UPDATE version SET version = 3;");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 3." << _db.databaseName();
    }
    return result;
}

#include "persistentstore.h"
#include "debug.h"

#include <QSettings>
#include <QFileInfo>
#include <QFile>

PersistentStore::PersistentStore(QObject *parent) : QObject(parent), _initialized(false)
{
    init();
}

bool PersistentStore::init()
{
    QMutexLocker locker(&_mutex);

    QSettings settings;
    QString appdata = settings.value("app/appdata").toString();
    if(appdata.isEmpty())
    {
        DEBUG << "Error creating the PersistentStore: app.appdata not set.";
        return false;
    }
    bool create = false;
    QString dbUrl(appdata + QDir::separator() + DEFAULT_DBNAME);
    QFileInfo dbFileInfo(dbUrl);

    _db = _db.addDatabase("QSQLITE");
    _db.setConnectOptions("QSQLITE_OPEN_URI");
    _db.setDatabaseName(dbUrl);

    if(!dbFileInfo.exists())
    {
        create = true;
    }
    else if(!dbFileInfo.isFile() || !dbFileInfo.isReadable())
    {
        DEBUG << "Error creating the PersistentStore: db file is not accessible " << dbUrl;
        return false;
    }
    else if (!_db.open())
    {
        DEBUG << "Error opening the PersistentStore db: " << _db.lastError().text();
        return false;
    }
    else
    {
        QStringList tables = _db.tables();
        if (!tables.contains("archives", Qt::CaseInsensitive))
        {
            _db.close();
            DEBUG << "Invalid PersistentStore db found. Attempting to recover.";
            if(!QFile::rename(dbUrl, dbUrl + "." + QString::number(QDateTime::currentMSecsSinceEpoch())))
            {
                DEBUG << "Failed to rename current invalid PersistentStore db. Please cleanup the dbs found in " << appdata;
                return false;
            }
            create = true;
        }
    }

    if(create)
    {
        QFile dbTemplate(":/dbtemplate.db");
        if(!dbTemplate.copy(dbUrl))
        {
            DEBUG << "Failed to create the PersistentStore db.";
            return false;
        }
        // Work around the fact that QFile::copy from the resource system does not set u+w on the resulted file
        QFile dbFile(dbUrl);
        dbFile.setPermissions(dbFile.permissions()|QFileDevice::WriteOwner);
        dbFile.close();
        if (!_db.open())
        {
            DEBUG << "Error opening the PersistentStore db: " << _db.lastError().text();
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
        _db.removeDatabase("QSQLITE");
        _initialized = false;
    }
}
QMutex* PersistentStore::mutex()
{
    return &_mutex;
}

QSqlQuery PersistentStore::createQuery()
{
    if (_initialized)
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



#include "persistentmodel/persistentstore.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <Qt>

static QMutex mutex;
WARNINGS_ENABLE

#include "debug.h"

#include "TSettings.h"

#include "upgrade-store.h"

#define DEFAULT_DBNAME "tarsnap.db"

PersistentStore *global_store = nullptr;

bool PersistentStore::_initialized = false;

void PersistentStore::initializePersistentStore()
{
    if(global_store == nullptr)
        global_store = new PersistentStore();
}

void PersistentStore::destroy()
{
    // This will happen if init_shared.cpp produces INIT_NEEDS_SETUP -- it
    // bails early from init_shared().
    if(global_store == nullptr)
        return;

    // Clean up.
    delete global_store;
    global_store = nullptr;
}

PersistentStore::PersistentStore()
{
}

bool PersistentStore::init()
{
    QMutexLocker locker(&mutex);

    // Get application data directory and database filename.
    TSettings settings;
    QString   appdata = settings.value("app/app_data", "").toString();
    if(appdata.isEmpty())
    {
        DEBUG << "Error creating the PersistentStore: app.appdata not set.";
        return false;
    }
    bool      create = false;
    QString   dbUrl(appdata + QDir::separator() + DEFAULT_DBNAME);
    QFileInfo dbFileInfo(dbUrl);

    // Initialize database object.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "tarsnap");
    db.setConnectOptions("QSQLITE_OPEN_URI");
    db.setDatabaseName(dbUrl);

    // Determine whether to try to open the database.
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
    } // Database file exists and is readable; attempt to open.
    else if(!db.open())
    {
        DEBUG << "Error opening the PersistentStore DB: "
              << db.lastError().text();
        return false;
    }
    else
    {
        // Successfully opened database.
        QStringList tables = db.tables();
        // Is the database valid?
        if(!tables.contains("archives", Qt::CaseInsensitive))
        {
            db.close();
            DEBUG << "Invalid PersistentStore DB found. Attempting to recover.";
            QString newName(
                dbUrl + "."
                + QString::number(QDateTime::currentMSecsSinceEpoch()));
            if(!QFile::rename(dbUrl, newName))
            {
                DEBUG << "Failed to rename current invalid PersistentStore DB. "
                         "Please manually clean up the DB directory "
                      << appdata;
                return false;
            }
            create = true;
        }
        else
        {
            // Check the database version, and upgrade if necessary.
            if(!upgrade_store(db, appdata))
                return false;
        }
    }

    // Create new database (if needed).
    if(create)
    {
        QFile dbTemplate(":/dbtemplate.db");

        // Create the directory for the db (if necessary)
        QDir dbDir = dbFileInfo.absoluteDir();
        if(!dbDir.exists())
            dbDir.mkpath(dbDir.absolutePath());

        if(!dbTemplate.exists())
            DEBUG << "DB template not found; check the resources file";
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
        if(!db.open())
        {
            DEBUG << "Error opening the PersistentStore DB: "
                  << db.lastError().text();
            return false;
        }
    }
    return _initialized = true;
}

void PersistentStore::deinit()
{
    QMutexLocker locker(&mutex);

    if(_initialized)
    {
        QSqlDatabase::removeDatabase("tarsnap");
        _initialized = false;
    }
}

QSqlQuery PersistentStore::createQuery()
{
    if(_initialized)
    {
        QSqlDatabase db = QSqlDatabase::database("tarsnap");
        return QSqlQuery(db);
    }
    else
    {
        DEBUG << "PersistentStore not initialized.";
        return QSqlQuery();
    }
}

void PersistentStore::purge()
{
    if(_initialized)
    {
        QString dbUrl;
        {
            QSqlDatabase db = QSqlDatabase::database("tarsnap");
            dbUrl           = db.databaseName();
        }
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

bool PersistentStore::runQuery(QSqlQuery query)
{
    QMutexLocker locker(&mutex);

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

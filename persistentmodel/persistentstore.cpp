#include "persistentstore.h"
#include "debug.h"

#include <QSettings>
#include <QFileInfo>
#include <QFile>

bool            PersistentStore::_initialized = false;
QSqlDatabase    PersistentStore::_db          = QSqlDatabase();

PersistentStore::PersistentStore(QObject *parent) : QObject(parent)
{
}

void PersistentStore::initialize()
{
    QSettings settings;
    QString appdata = settings.value("app/appdata").toString();
    if(appdata.isEmpty())
    {
        DEBUG << "Error creating the PersistentStore: app.appdata not set.";
        return;
    }
    bool create = false;
    QString dbUrl(appdata + QDir::separator() + DEFAULT_DBNAME);
    QFileInfo dbFileInfo(dbUrl);

    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setConnectOptions("QSQLITE_OPEN_URI");
    _db.setDatabaseName(dbUrl);

    if(!dbFileInfo.exists())
    {
        create = true;
    }
    else if(!dbFileInfo.isFile() || !dbFileInfo.isReadable())
    {
        DEBUG << "Error creating the PersistentStore: db file is not accessible " << dbUrl;
        return;
    }
    else if (!_db.open())
    {
        DEBUG << "Error opening the PersistentStore db: " << _db.lastError().text();
        return;
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
                return;
            }
            create = true;
        }
    }

    if(create)
    {
        QFile dbTemplate(":/resources/dbtemplate.db");
        if(!dbTemplate.copy(dbUrl))
        {
            DEBUG << "Failed to create the PersistentStore db.";
            return;
        }
        // Work around the fact that QFile::copy from the resource system does not set u+w on the resulted file
        QFile dbFile(dbUrl);
        dbFile.setPermissions(dbFile.permissions()|QFileDevice::WriteOwner);
        if (!_db.open())
        {
            DEBUG << "Error opening the PersistentStore db: " << _db.lastError().text();
            return;
        }
    }

    _initialized = true;

    //        QString query(dbTemplate.readAll());
    //        DEBUG << query;
    //        QSqlQuery q;
    //        if (!q.exec(query))
    //        {
    //            DEBUG << "Error creating the PersistentStore: " << q.lastError().text();
    //            return;
    //        }
    QSqlQuery q;
    if (!q.exec(QLatin1String("create table test_table(id integer primary key, name varchar)")))
        DEBUG << q.lastError();
}

PersistentStore::~PersistentStore()
{
    _db.close();
    QSqlDatabase::removeDatabase("QSQLITE");
    _initialized = false;
}


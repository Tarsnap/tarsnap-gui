#include "upgrade-store.h"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include "debug.h"

/* Forward declaration(s). */
static bool upgradeVersion0();
static bool upgradeVersion1();
static bool upgradeVersion2();
static bool upgradeVersion3();
static bool upgradeVersion4();

bool upgrade_store(QSqlDatabase db, const QString &appdata)
{
    QStringList tables = db.tables();

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
    QSqlQuery query(db);
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
    if((version == 3) && upgradeVersion4())
    {
        DEBUG << "DB upgraded to version 4.";
        version = 4;
    }
    (void)version; /* not used beyond this point. */
    return true;
}

/*
 * The below "if((result =" lines don't play well with clang-format.
 * I intend to rewrite them once I have a test suite for checking the
 * version upgrading.
 */
/* clang-format off */
static bool upgradeVersion0()
{
    bool      result = false;
    QSqlDatabase db = QSqlDatabase::database("tarsnap");
    QSqlQuery query(db);

    if((result = query.exec("CREATE TABLE version (version INTEGER NOT NULL);")))
        result = query.exec("INSERT INTO version VALUES (0);");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 0." << db;
    }
    return result;
}

static bool upgradeVersion1()
{
    bool      result = false;
    QSqlDatabase db = QSqlDatabase::database("tarsnap");
    QSqlQuery query(db);

    if((result = query.exec("ALTER TABLE jobs ADD COLUMN optionScheduledEnabled INTEGER;")))
        result = query.exec("UPDATE version SET version = 1;");

    // Handle the special case, since I started versioning DB after two app
    // releases and this change in between...
    if(!result && query.lastError().text().contains("duplicate column name"))
        result = query.exec("UPDATE version SET version = 1;");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 1." << db.databaseName();
    }
    return result;
}

static bool upgradeVersion2()
{
    bool      result = false;
    QSqlDatabase db = QSqlDatabase::database("tarsnap");
    QSqlQuery query(db);

    if((result = query.exec("ALTER TABLE jobs ADD COLUMN optionSkipFiles INTEGER;")))
    if((result = query.exec("ALTER TABLE jobs ADD COLUMN optionSkipFilesPatterns TEXT;")))
        result = query.exec("UPDATE version SET version = 2;");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 2." << db.databaseName();
    }
    return result;
}

static bool upgradeVersion3()
{
    bool      result = false;
    QSqlDatabase db = QSqlDatabase::database("tarsnap");
    QSqlQuery query(db);

    if((result = query.exec("ALTER TABLE jobs ADD COLUMN optionSkipNoDump INTEGER;")))
    if((result = query.exec("UPDATE archives SET contents=\"\";")))
    if((result = query.exec("CREATE TABLE journal (timestamp INTEGER NOT NULL, log TEXT);")))
        result = query.exec("UPDATE version SET version = 3;");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 3." << db.databaseName();
    }
    return result;
}

static bool upgradeVersion4()
{
    bool      result = false;
    QSqlDatabase db = QSqlDatabase::database("tarsnap");
    QSqlQuery query(db);

    if((result = query.exec("UPDATE archives SET contents=\"\";")))
    if((result = query.exec("ALTER TABLE archives ADD COLUMN truncated INTEGER;")))
    if((result = query.exec("ALTER TABLE archives ADD COLUMN truncatedInfo TEXT;")))
    if((result = query.exec("ALTER TABLE jobs ADD COLUMN settingShowHidden INTEGER;")))
    if((result = query.exec("ALTER TABLE jobs ADD COLUMN settingShowSystem INTEGER;")))
    if((result = query.exec("ALTER TABLE jobs ADD COLUMN settingHideSymlinks INTEGER;")))
    if((result = query.exec("UPDATE jobs SET optionScheduledEnabled=0;")))
        result = query.exec("UPDATE version SET version = 4;");

    if(!result)
    {
        DEBUG << query.lastError().text();
        DEBUG << "Failed to upgrade DB to version 4." << db.databaseName();
    }
    return result;
}
/* clang-format on */

#include <QSqlDatabase>

#include "../compile-valgrind-test.h"

void pl_nothing()
{
}

void pl_QSqlDatabase()
{
    QString connectionName;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        connectionName  = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_QSqlDatabase)
T_TEST_END

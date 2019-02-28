#include <QSqlDatabase>

#include "../compile-valgrind-test.h"
#include "../console-test.h"

static void pl_nothing()
{
}

static void pl_QSqlDatabase()
{
    T_TEST_CONSOLE_BEGIN

    QString connectionName;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        connectionName  = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    T_TEST_CONSOLE_END
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_QSqlDatabase)
T_TEST_END

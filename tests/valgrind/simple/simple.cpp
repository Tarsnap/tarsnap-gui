#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QEventLoop>
#include <QSqlDatabase>
WARNINGS_ENABLE

#include "../macros-valgrind.h"

static void pl_nothing()
{
}

static void pl_QSqlDatabase()
{
    T_APP_BEGIN_CONSOLE;

    QString connectionName;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        connectionName  = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    T_APP_END;
}

static void pl_processEvents()
{
    T_APP_BEGIN_CONSOLE;

    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

    T_APP_END;
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_QSqlDatabase),
    MEMLEAKTEST(pl_processEvents)
T_TEST_END

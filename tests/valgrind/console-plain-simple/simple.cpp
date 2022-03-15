#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QEventLoop>
#include <QSqlDatabase>
#include <QTextCodec>
WARNINGS_ENABLE

#include "../macros-valgrind.h"

static void pl_nothing()
{
}

static void pl_no_app_codec()
{
    // To trigger this leak, do *not* use T_APP_BEGIN_CONSOLE.
    // This is arguably an unfair case, since a lot of Qt is built on the
    // assumption that there exists exactly 1 QCoreApplication or QApplication.
    // However, there's no such listed dependency in the docs for QTextCodec,
    // and this leak occurs whenever you print anything to the console.

    QTextCodec::codecForLocale();

    // To trigger this leak, do *not* use T_APP_END.
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

static void pl_exec()
{
    T_APP_BEGIN_CONSOLE;

    // Queue a quit event.
    QMetaObject::invokeMethod(app, "quit", Qt::QueuedConnection);

    // Launch the event loop.
    app->exec();

    T_APP_END;
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_no_app_codec),
    MEMLEAKTEST(pl_QSqlDatabase),
    MEMLEAKTEST(pl_processEvents),
    MEMLEAKTEST(pl_exec)
T_TEST_END

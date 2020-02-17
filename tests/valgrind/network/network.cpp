#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QThread>
WARNINGS_ENABLE

#include "../macros-valgrind.h"

static void pl_nothing()
{
}

static void pl_networkaccessmanager()
{
    T_APP_BEGIN_CONSOLE;

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    delete nam;

    T_APP_END;
}

static void pl_networkaccessmanager_wait()
{
    T_APP_BEGIN_CONSOLE;

    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QThread::msleep(200);
    delete nam;

    T_APP_END;
}

static void pl_networkaccessmanager_status_not_solo()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager();

    QThread::msleep(200);
    nam->networkAccessible();
    QThread::msleep(200);

    delete nam;
}

static void pl_networkaccessmanager_status()
{
    T_APP_BEGIN_CONSOLE;

    pl_networkaccessmanager_status_not_solo();

    T_APP_END;
}

static void pl_networkaccessmanager_repeated()
{
    T_APP_BEGIN_CONSOLE;

    // QNetworkAccessManger has an unpredictable interactions with dbus,
    // causing variable memory leaks.  This is my attempt at catching them.
    for(int i = 0; i < 10; i++)
    {
        pl_networkaccessmanager_status_not_solo();
        QThread::msleep(200);
    }

    T_APP_END;
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_networkaccessmanager),
    MEMLEAKTEST(pl_networkaccessmanager_wait),
    MEMLEAKTEST(pl_networkaccessmanager_status),
    MEMLEAKTEST(pl_networkaccessmanager_repeated)
T_TEST_END

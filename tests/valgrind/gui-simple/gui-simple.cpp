#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QAccessible>
#include <QApplication>
#include <QIcon>
#include <QMetaObject>
#include <QObject>
#include <QPixmap>
#include <QWidget>
WARNINGS_ENABLE

#include "../macros-valgrind.h"

static void pl_nothing()
{
}

static void pl_app()
{
    T_APP_BEGIN_GUI;

    T_APP_END_GUI;
}

static void pl_object()
{
    // Create object before app
    QObject *obj = new QObject();

    T_APP_BEGIN_GUI;

    T_APP_END_GUI;

    // Delete object after app
    delete obj;
}

static void pl_exec()
{
    T_APP_BEGIN_GUI;

    // Queue a quit event.
    QMetaObject::invokeMethod(app, "quit", Qt::QueuedConnection);

    // Launch the event loop.
    app->exec();

    T_APP_END_GUI;
}

static void pl_icon()
{
    T_APP_BEGIN_GUI;

    QIcon *icon = new QIcon();
    icon->addFile("../16x16-white.png");
    delete icon;

    T_APP_END_GUI;
}

static void pl_app_pix()
{
    T_APP_BEGIN_GUI;

    QWidget *wid = new QWidget();
    QPixmap *pix = new QPixmap("../16x16-white.png");
    delete pix;
    delete wid;

    T_APP_END_GUI;
}

static void pl_app_pix_two()
{
    pl_app_pix();
    pl_app_pix();
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_app),
    MEMLEAKTEST(pl_object),
    MEMLEAKTEST(pl_exec),
    MEMLEAKTEST(pl_icon),
    MEMLEAKTEST(pl_app_pix_two)
T_TEST_END

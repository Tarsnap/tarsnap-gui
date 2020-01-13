#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QAccessible>
#include <QApplication>
#include <QObject>
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

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_app),
    MEMLEAKTEST(pl_object)
T_TEST_END

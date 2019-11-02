#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
WARNINGS_ENABLE

#include "../macros-valgrind.h"

static void pl_nothing()
{
}

static void pl_QCoreApplication()
{
    T_APP_BEGIN_CONSOLE;

    T_APP_END;
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_QCoreApplication)
T_TEST_END

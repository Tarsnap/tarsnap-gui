#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
WARNINGS_ENABLE

#include "../compile-valgrind-test.h"
#include "../macros-valgrind.h"

static void pl_nothing()
{
}

static void pl_QCoreApplication()
{
    T_TEST_CONSOLE_BEGIN

    T_TEST_CONSOLE_END
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_QCoreApplication)
T_TEST_END

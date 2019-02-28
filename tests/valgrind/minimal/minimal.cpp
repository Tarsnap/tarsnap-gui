#include <QCoreApplication>

#include "../compile-valgrind-test.h"
#include "../console-test.h"

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

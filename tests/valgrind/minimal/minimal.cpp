#include <QCoreApplication>

#include "../compile-valgrind-test.h"

void pl_nothing()
{
}

void pl_QCoreApplication()
{
    // Set up variables
    int   argc = 1;
    char *argv[1];
    argv[0] = strdup("arg");

    // Actual memory test
    QCoreApplication *app = new QCoreApplication(argc, argv);
    delete app;

    // Clean up variable
    free(argv[0]);
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_QCoreApplication)
T_TEST_END

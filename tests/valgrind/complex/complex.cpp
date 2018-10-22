#include <QCoreApplication>

#include "app.h"

#include "../compile-valgrind-test.h"

void pl_nothing()
{
}

void pl_processEvents()
{
    // Set up variables
    int   argc = 1;
    char *argv[1];
    argv[0] = strdup("app");

    // Actual memory test
    App *app = new App(argc, argv);
    app->process();
    delete app;

    // Clean up variable
    free(argv[0]);
}

// clang-format off
T_TEST_BEGIN
    MEMLEAKTEST(pl_nothing),
    MEMLEAKTEST(pl_processEvents)
T_TEST_END

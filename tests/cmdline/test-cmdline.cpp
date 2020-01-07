#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "app-cmdline.h"
#include "init-shared.h"
#include "persistentmodel/persistentstore.h"

#include <TSettings.h>

extern "C" {
#include "optparse.h"
WARNINGS_DISABLE
#include "warnp.h"
WARNINGS_ENABLE
}

class TestCmdline : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void normal_init();
    void appdir_init();
};

void TestCmdline::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    HANDLE_IGNORING_XDG_HOME;

    // Initialize debug messages.
    const char *argv[] = {"test-cmdline"};
    WARNP_INIT;
}

void TestCmdline::normal_init()
{
    struct optparse *opt;

    // Create command-line arguments
    int   argc = 1;
    char *argv[1];
    argv[0] = strdup("./test-cmdline");

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == nullptr)
        QVERIFY(opt != nullptr);

    // This scope is how we do it in main.cpp
    {
        // Basic initialization that cannot fail.
        AppCmdline app(argc, argv, opt);

        // Run more complicated initialization.
        if(!app.initializeCore())
            QFAIL("Could not initialize app");
    }

    // Check that it read the right config file.
    TSettings settings;
    QVERIFY(settings.value("tarsnap/user", "") == "normal_init");

    // Clean up
    PersistentStore::deinit();
    init_shared_free();
    optparse_free(opt);
    free(argv[0]);
}

void TestCmdline::appdir_init()
{
    struct optparse *opt;

    // Create command-line arguments
    int   argc = 3;
    char *argv[3];
    argv[0] = strdup("./test-cmdline");
    argv[1] = strdup("--appdata");
    argv[2] = strdup("confdir");

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == nullptr)
        QVERIFY(opt != nullptr);

    // This scope is how we do it in main.cpp
    {
        // Basic initialization that cannot fail.
        AppCmdline app(argc, argv, opt);

        // Run more complicated initialization.
        if(!app.initializeCore())
            QFAIL("Could not initialize app");
    }

    // Check that it read the right config file.
    TSettings settings;
    QVERIFY(settings.value("tarsnap/user", "") == "appdata_init");

    // Clean up
    PersistentStore::deinit();
    init_shared_free();
    optparse_free(opt);
    free(argv[0]);
    free(argv[1]);
    free(argv[2]);
}

QTEST_MAIN(TestCmdline)
#include "test-cmdline.moc"

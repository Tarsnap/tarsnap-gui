#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "app-cmdline.h"
#include "persistentmodel/persistentstore.h"

#include <TSettings.h>

extern "C" {
#include "optparse.h"
#include "warnp.h"
}

class TestCmdline : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanupTestCase();

    void normal_init();
    void appdir_init();
};

void TestCmdline::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Initialize debug messages.
    const char *argv[] = {"test-cmdline"};
    WARNP_INIT;
}

void TestCmdline::init()
{
    // Reset TSettings
    TSettings::destroy();

    // Reset PersistentStore
    PersistentStore::deinit();
}

void TestCmdline::cleanupTestCase()
{
    TSettings::destroy();
}

void TestCmdline::normal_init()
{
    struct optparse *opt;

    HANDLE_IGNORING_XDG_HOME;

    // Create command-line arguments
    int   argc = 1;
    char *argv[1];
    argv[0] = strdup("./test-cmdline");

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == NULL)
        QVERIFY(opt != NULL);

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
    if((opt = optparse_parse(argc, argv)) == NULL)
        QVERIFY(opt != NULL);

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
    optparse_free(opt);
    free(argv[0]);
    free(argv[1]);
    free(argv[2]);
}

QTEST_MAIN(TestCmdline)
#include "test-cmdline.moc"

#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "app-cmdline.h"

extern "C" {
#include "optparse.h"
#include "warnp.h"
}

class TestCmdline : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    void normal_init();
#endif
    void appdir_init();
};

void TestCmdline::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Initialize debug messages.
    const char *argv[] = {"test-cmdline"};
    WARNP_INIT;
}

// FIXME: this is not yet cross-platform because OSX uses a .plist file.  That
// will be fixed soon with the upcoming TSettings change.
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
void TestCmdline::normal_init()
{
    struct optparse *opt;

    // Create command-line arguments
    int   argc = 1;
    char *argv[1];
    argv[0] = strdup("./test-cmdline");

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == NULL)
        QVERIFY(opt != NULL);

    // Basic initialization that cannot fail.
    AppCmdline app(argc, argv, opt);
    optparse_free(opt);

    // Run more complicated initialization.
    if(!app.initializeCore())
        QFAIL("Could not initialize app");

    // Check that it read the right config file.
    QSettings settings;
    QVERIFY(settings.value("tarsnap/user") == "normal_init");
}
#endif

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

    // Basic initialization that cannot fail.
    AppCmdline app(argc, argv, opt);
    optparse_free(opt);

    // Run more complicated initialization.
    if(!app.initializeCore())
        QFAIL("Could not initialize app");

    // Check that it read the right config file.
    QSettings settings;
    QVERIFY(settings.value("tarsnap/user") == "appdata_init");
}

QTEST_MAIN(TestCmdline)
#include "test-cmdline.moc"

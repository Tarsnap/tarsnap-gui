#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QList>
#include <QObject>
WARNINGS_ENABLE

#include <stdlib.h>
#include <string.h>

#include "../qtest-platform.h"

#include "app-cmdline.h"
#include "init-shared.h"

#include "TSettings.h"

extern "C" {
#include "optparse.h"
WARNINGS_DISABLE
#include "warnp.h"
WARNINGS_ENABLE
}

class TestAppCmdline : public QObject
{
    Q_OBJECT

    /* We're not running this within QTest. */
public:
    void initTestCase();

    void normal_init();
    void appdir_init();
};

void TestAppCmdline::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // QTest normally takes care of ::setApplicationName.
    QCoreApplication::setApplicationName(TEST_NAME);

    HANDLE_IGNORING_XDG_HOME;

    // Initialize debug messages.
    const char *argv[] = {"test-app-cmdline"};
    WARNP_INIT;
}

void TestAppCmdline::normal_init()
{
    struct optparse *opt;

    // Create command-line arguments
    int   argc = 1;
    char *argv[1];
    argv[0] = strdup("./test-app-cmdline");

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == nullptr)
    {
        warn0("optparse_parse");
        exit(1);
    }

    // This scope is how we do it in main.cpp
    {
        const QList<struct init_info> steps = init_shared(opt->config_dir);

        // Basic initialization that cannot fail.
        AppCmdline app(argc, argv, opt);

        // Act on any initialization failures
        if(!app.handle_init(steps))
        {
            warn0("Could not initialize app");
            exit(1);
        }
    }

    // Check that it read the right config file.
    TSettings settings;
    if(settings.value("tarsnap/user", "") != "normal_init")
    {
        warn0("failed normal_init");
        exit(1);
    }

    // Clean up
    init_shared_free();
    optparse_free(opt);
    free(argv[0]);
}

void TestAppCmdline::appdir_init()
{
    struct optparse *opt;

    // Create command-line arguments
    int   argc = 3;
    char *argv[3];
    argv[0] = strdup("./test-app-cmdline");
    argv[1] = strdup("--appdata");
    argv[2] = strdup("confdir");

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == nullptr)
    {
        warn0("optparse_parse");
        exit(1);
    }

    // This scope is how we do it in main.cpp
    {
        const QList<struct init_info> steps = init_shared(opt->config_dir);

        // Basic initialization that cannot fail.
        AppCmdline app(argc, argv, opt);

        // Act on any initialization failures
        if(!app.handle_init(steps))
        {
            warn0("Could not initialize app");
            exit(1);
        }
    }

    // Check that it read the right config file.
    TSettings settings;
    if(settings.value("tarsnap/user", "") != "appdata_init")
    {
        warn0("failed appdata_init");
        exit(1);
    }

    // Clean up
    init_shared_free();
    optparse_free(opt);
    free(argv[0]);
    free(argv[1]);
    free(argv[2]);
}

/*
 * We're using an explicit main() so that QTest won't make its own
 * QCoreApplication, as we're only allowed to have one at once.
 */
int main(int argc, char *argv[])
{
    TestAppCmdline *tac = new TestAppCmdline();

    /* Basic initialization. */
    tac->initTestCase();

    if(argc > 1)
    {
        warn0("Command-line arguments not supported in this test.");
        exit(1);
    }
    (void)argv; /* UNUSED */

    /* Run tests. */
    tac->normal_init();
    tac->appdir_init();

    /* Clean up. */
    delete tac;

    /* Success! */
    return (0);
}

WARNINGS_DISABLE
#include "test-app-cmdline.moc"
WARNINGS_ENABLE

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAbstractButton>
#include <QCoreApplication>
#include <QLineEdit>
#include <QList>
#include <QObject>
#include <QString>
#include <QTest>

#include "ui_setupwizard_cli.h"
#include "ui_setupwizard_register.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "messages/taskstatus.h"

#include "app-setup.h"
#include "dir-utils.h"
#include "init-shared.h"
#include "setupwizard/setupwizard.h"
#include "setupwizard/setupwizard_cli.h"
#include "setupwizard/setupwizard_register.h"
#include "tasks/tasks-defs.h"

#include "TPathLineBrowse.h"
#include "TSettings.h"
#include "TWizardPage.h"

extern "C" {
#include "optparse.h"
WARNINGS_DISABLE
#include "warnp.h"
WARNINGS_ENABLE
}

#define GET_UI_PAGE(x, PAGETYPE)                                               \
    do                                                                         \
    {                                                                          \
        const TWizardPage *page = wizard->currentPage();                       \
        QVERIFY(page->objectName() == #PAGETYPE);                              \
        x = static_cast<const PAGETYPE *>(page)->get_ui();                     \
    } while(0)

#define GET_BUTTON(x) wizard->currentPage()->button(TWizardPage::x)

/* "Runner" functions, which control the AppSetup. */
static void run_cancel(AppSetup *setup);
static void run_normal_setup(AppSetup *setup);

/* Infrastructure. */
static void setup(int argc, char *argv[], void(runner)(AppSetup *setup),
                  int expected_ret);
static void init();

static void run_cancel(AppSetup *setup)
{
    SetupWizard *wizard = setup->get_wizard();

    // Intro page
    VISUAL_WAIT;
    wizard->close();
}

static void run_normal_setup(AppSetup *setup)
{
    SetupWizard *wizard = setup->get_wizard();

    Ui::CliPage      *ui_cli;
    Ui::RegisterPage *ui_register;

    // Check if we have tarsnap (without using QTest's QSKIP).
    struct DirMessage cliDirMessage =
        findTarsnapClientInPath(QString(""), true);
    QString tarsnapPath = cliDirMessage.dirname;

    // Intro page
    QCOMPARE(wizard->pageTitle(), QStringLiteral("Setup wizard"));
    VISUAL_WAIT;
    GET_BUTTON(NextButton)->click();

    // CLI page
    QCOMPARE(wizard->pageTitle(), QStringLiteral("Command-line utilities"));
    GET_UI_PAGE(ui_cli, CliPage);
    VISUAL_WAIT;

    if(tarsnapPath.isEmpty())
    {
        ui_cli->cliPathLineBrowse->setText("faked-dir");
        // Force the settings to contain the faked binary dir.
        TSettings settings;
        settings.setValue("tarsnap/path", "faked-dir");
        // Fake the binary.
        wizard->tarsnapVersionResponse(TaskStatus::Completed,
                                       TARSNAP_MIN_VERSION);
        VISUAL_WAIT;
    }
    setup->waitUntilIdle();
    GET_BUTTON(NextButton)->click();

    // Register page
    QCOMPARE(wizard->pageTitle(), QStringLiteral("Register with server"));
    GET_UI_PAGE(ui_register, RegisterPage);
    setup->fakeNextTask();
    ui_register->tarsnapUserLineEdit->setText("email@example.org");
    ui_register->tarsnapPasswordLineEdit->setText("hunter2");
    VISUAL_WAIT;

    // Add an extra check here so that we can debug any "Next button" failure.
    QCOMPARE(GET_BUTTON(NextButton)->isEnabled(), true);
    GET_BUTTON(NextButton)->click();

    // Wait for the "Register machine" process (with Qt::QueuedConnection) to
    // finish.
    setup->waitUntilIdle();

    // Final page
    QCOMPARE(wizard->pageTitle(), QStringLiteral("Setup complete!"));
    VISUAL_WAIT;
    GET_BUTTON(FinishButton)->click();

    VISUAL_WAIT;
}

static void setup(int argc, char *argv[], void(runner)(AppSetup *setup),
                  int expected_ret)
{
    struct optparse *opt;

    // Parse command-line arguments.
    if((opt = optparse_parse(argc, argv)) == nullptr)
        QVERIFY(opt != nullptr);

    // This scope is how we do it in main.cpp.
    {
        const QList<struct init_info> steps = init_shared(opt->config_dir);

        // Basic initialization that cannot fail.
        AppSetup app(argc, argv);
        QObject::connect(&app, &AppSetup::eventLoopStarted, runner);

        // Use a custom message handler to filter out unwanted messages
        IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }

        // Act on any initialization failures.
        if(!app.handle_init(steps))
            QFAIL("Could not initialize app");

        // Prepare and launch event loop.
        if(app.prepEventLoop())
        {
            int ret = app.exec();

            // Check result.
            QCOMPARE(ret, expected_ret);
        }
    }

    // Clean up.
    init_shared_free();
    optparse_free(opt);
}

static void init()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // QTest normally takes care of ::setApplicatonName.
    QCoreApplication::setApplicationName(TEST_NAME);

    HANDLE_IGNORING_XDG_HOME;

    // Initialize debug messages.
    const char *argv[] = {"test-setup"};
    WARNP_INIT;
}

int main(int argc, char *argv[])
{
    init();

    setup(argc, argv, run_cancel, 1);
    setup(argc, argv, run_normal_setup, 0);

    return (0);
}

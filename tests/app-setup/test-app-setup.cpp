#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QMetaObject>
#include <QtTest/QtTest>

#include "ui_setupdialog.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "app-setup.h"
#include "init-shared.h"
#include "setupdialog.h"
#include "tasks-defs.h"
#include "taskstatus.h"

#include <TSettings.h>

extern "C" {
#include "optparse.h"
WARNINGS_DISABLE
#include "warnp.h"
WARNINGS_ENABLE
}

/* "Runner" functions, which control the AppSetup. */
static void run_cancel(AppSetup *setup);
static void run_normal_setup(AppSetup *setup);

/* Infrastructure. */
static void setup(int argc, char *argv[], void(runner)(AppSetup *setup),
                  int expected_ret);
static void init();

static void run_cancel(AppSetup *setup)
{
    SetupDialog *wizard = setup->get_wizard();

    // Intro page
    VISUAL_WAIT;
    wizard->close();
}

static void run_normal_setup(AppSetup *setup)
{
    SetupDialog *    wizard = setup->get_wizard();
    Ui::SetupDialog *ui     = wizard->get_ui();

    // Check if we have tarsnap (without using QTest's QSKIP).
    QString tarsnapPath = Utils::findTarsnapClientInPath(QString(""), true);

    // Intro page
    VISUAL_WAIT;
    wizard->next();

    // CLI page
    Q_ASSERT(ui->titleLabel->text() == "Command-line utilities");
    VISUAL_WAIT;
    if(tarsnapPath.isEmpty())
    {
        ui->tarsnapPathLineEdit->setText("faked-dir");
        // Fake the binary.
        wizard->tarsnapVersionResponse(TaskStatus::Completed,
                                       TARSNAP_MIN_VERSION);
        VISUAL_WAIT;
    }
    setup->waitUntilIdle();
    wizard->next();

    // Register page
    Q_ASSERT(ui->titleLabel->text() == "Register with server");
    setup->fakeNextTask();
    ui->tarsnapUserLineEdit->setText("email@example.org");
    ui->tarsnapPasswordLineEdit->setText("hunter2");
    VISUAL_WAIT;
    wizard->next();

    // Wait for the "Register machine" process (with Qt::QueuedConnection) to
    // finish.
    setup->waitUntilIdle();

    // Final page
    Q_ASSERT(ui->titleLabel->text() == "Setup complete!");
    VISUAL_WAIT;
    wizard->next();

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

        IF_NOT_VISUAL
        {
            // Use a custom message handler to filter out unwanted messages
            orig_message_handler =
                qInstallMessageHandler(offscreenMessageOutput);
        }

        // Act on any initialization failures.
        if(!app.handle_init(steps))
            QFAIL("Could not initialize app");

        // Prepare and launch event loop.
        if(app.prepEventLoop())
        {
            int ret = app.exec();

            // Check result.
            Q_ASSERT(ret == expected_ret);
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

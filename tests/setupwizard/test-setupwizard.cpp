#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include <QSettings>
#include <QSignalSpy>
#include <QString>
#include <QTabWidget>
#include <QTest>
#include <Qt>

#include "ui_setupwizard_cli.h"
#include "ui_setupwizard_register.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "TElidedLabel.h"
#include "TPathComboBrowse.h"
#include "TPathLineBrowse.h"
#include "TSettings.h"
#include "TWizardPage.h"

#include "messages/taskstatus.h"

#include "compare-settings.h"

#include "setupwizard/setupwizard.h"
#include "setupwizard/setupwizard_cli.h"
#include "setupwizard/setupwizard_register.h"

#define GET_UI_PAGE(x, PAGETYPE)                                               \
    do                                                                         \
    {                                                                          \
        const TWizardPage *page = wizard->currentPage();                       \
        QVERIFY(page->objectName() == #PAGETYPE);                              \
        x = static_cast<const PAGETYPE *>(page)->get_ui();                     \
    } while(0)

#define GET_BUTTON(x) wizard->currentPage()->button(TWizardPage::x)

class TestSetupWizard : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void do_nothing();
    void cli();
    void version_too_low();
    void cancel_install();
    void skip_install();
    void normal_install();

private:
    void helper_almost_normal_install(SetupWizard *wizard);
};

void TestSetupWizard::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }
}

void TestSetupWizard::cleanupTestCase()
{
    TSettings::destroy();

    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

static void clean_quit(SetupWizard *wizard)
{
    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;

    // Ensure that the wizard is not running when we quit
    if(wizard->isVisible())
        wizard->reject();

    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;

    delete wizard;
}

void TestSetupWizard::do_nothing()
{
    SetupWizard *wizard = new SetupWizard();

    // This test is intended to help debug memory leaks.
    VISUAL_INIT(wizard);

    clean_quit(wizard);
}

void TestSetupWizard::helper_almost_normal_install(SetupWizard *wizard)
{
    QSignalSpy sig_cli(wizard, SIGNAL(tarsnapVersionRequested()));
    QSignalSpy sig_register(wizard,
                            SIGNAL(registerMachineRequested(QString, QString,
                                                            bool)));

    Ui::CliPage      *ui_cli;
    Ui::RegisterPage *ui_register;

    VISUAL_INIT(wizard);

    // Wait for it to finish opening.
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

    // Page 1
    QVERIFY(wizard->pageTitle() == "Setup wizard");
    QVERIFY(GET_BUTTON(NextButton)->hasFocus());
    GET_BUTTON(NextButton)->click();
    VISUAL_WAIT;

    // Page 2
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    GET_UI_PAGE(ui_cli, CliPage);
    VISUAL_WAIT;

    // Fake the CLI detection and checking
    QVERIFY(sig_cli.count() == 1);
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(ui_cli->validationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    // Proceed
    GET_BUTTON(NextButton)->click();
    VISUAL_WAIT;

    // Page 3
    QVERIFY(wizard->pageTitle() == "Register with server");
    GET_UI_PAGE(ui_register, RegisterPage);

    // Pretend that we already have a key
    ui_register->keyfileTabWidget->setCurrentIndex(RegisterPage::UseKeyfileTab);
    ui_register->keyfilePathComboBrowse->setText("empty.key");
    VISUAL_WAIT;

    // Pretend to register
    GET_BUTTON(NextButton)->click();
    QVERIFY(sig_register.count() == 1);
    wizard->updateNumTasks(false, 1, 0);
    VISUAL_WAIT;

    // Fake a response
    wizard->registerMachineResponse(TaskStatus::Completed, "");
    VISUAL_WAIT;

    // Wait for the test to proceed (for up to 1 second)
    for(int i = 0; i < 10; i++)
    {
        if(wizard->pageTitle() == "Setup complete!")
            break;
        QTest::qWait(100);
    }

    // Page 4
    QVERIFY(wizard->pageTitle() == "Setup complete!");
    VISUAL_WAIT;
}

void TestSetupWizard::normal_install()
{
    TARSNAP_CLI_OR_SKIP;

    SetupWizard *wizard = new SetupWizard();

    // Almost complete a normal install
    helper_almost_normal_install(wizard);

    // Finish the install
    GET_BUTTON(FinishButton)->click();
    VISUAL_WAIT;

    // Check resulting init file.  The first can be in any format (for now).
    TSettings settings;
    QSettings target("after-normal-install.conf", QSettings::IniFormat);
    QVERIFY(compareSettings(settings.getQSettings(), &target));

    // Clean up
    clean_quit(wizard);
}

void TestSetupWizard::cancel_install()
{
    TARSNAP_CLI_OR_SKIP;

    SetupWizard *wizard = new SetupWizard();

    // Almost complete a normal install
    helper_almost_normal_install(wizard);

    // Close before we actually finish
    QTest::keyEvent(QTest::Click, wizard, Qt::Key_Escape);
    VISUAL_WAIT;

    // Clean up
    clean_quit(wizard);

    // Check that we wiped the Tarsnap-related settings
    TSettings  settings;
    QSettings *q = settings.getQSettings();
    q->beginGroup("tarsnap");
    QVERIFY(q->allKeys().length() == 0);
    q->beginGroup("app");
    QVERIFY(q->allKeys().length() == 0);
}

void TestSetupWizard::skip_install()
{
    TARSNAP_CLI_OR_SKIP;

    SetupWizard *wizard = new SetupWizard();

    // Almost complete a normal install
    helper_almost_normal_install(wizard);

    // Now go back to the beginning and skip the install
    GET_BUTTON(BackButton)->click();
    GET_BUTTON(BackButton)->click();
    GET_BUTTON(BackButton)->click();
    VISUAL_WAIT;

    GET_BUTTON(SkipButton)->click();
    VISUAL_WAIT;

    // Check resulting init file.
    TSettings settings;
    QSettings target("after-skip-install.conf", QSettings::IniFormat);
    QVERIFY(compareSettings(settings.getQSettings(), &target));

    // Clean up
    clean_quit(wizard);
}

void TestSetupWizard::cli()
{
    SetupWizard *wizard = new SetupWizard();
    Ui::CliPage *ui_cli;

    VISUAL_INIT(wizard);

    // Advance to CLI page and expand advanced options
    GET_BUTTON(NextButton)->click();
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    GET_UI_PAGE(ui_cli, CliPage);
    VISUAL_WAIT;

    // We may or may not receive a version query, depending on whether
    // tarsnap is installed, so don't check for this here.
    // (We check "did we receive a version-query signal" in other
    // tarsnap-dependent tests.)
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    VISUAL_WAIT;

    // App data directory
    ui_cli->appdataPathLineBrowse->setText("fake-dir");
    QVERIFY(ui_cli->appdataPathLineBrowse->statusText() == "Does not exist.");
    VISUAL_WAIT;
    ui_cli->appdataPathLineBrowse->setText("/tmp");

    // Cache directory
    ui_cli->cachePathLineBrowse->setText("fake-dir");
    QVERIFY(ui_cli->cachePathLineBrowse->statusText() == "Does not exist.");
    VISUAL_WAIT;
    ui_cli->cachePathLineBrowse->setText("/tmp");

    // Tarsnap CLI directory
    ui_cli->cliPathLineBrowse->setText("fake-dir");
    QVERIFY(ui_cli->cliPathLineBrowse->statusText() == "Executable not found.");
    VISUAL_WAIT;
    ui_cli->cliPathLineBrowse->setText("/tmp");

    // Fake detecting the binaries
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(ui_cli->validationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    // Ensure that the wizard is not running when we quit
    clean_quit(wizard);
}

void TestSetupWizard::version_too_low()
{
    SetupWizard *wizard = new SetupWizard();
    Ui::CliPage *ui_cli;

    VISUAL_INIT(wizard);

    // Advance to CLI page
    GET_BUTTON(NextButton)->click();
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    GET_UI_PAGE(ui_cli, CliPage);
    VISUAL_WAIT;

    // Fake detecting the binaries with a too-low version number
    wizard->tarsnapVersionResponse(TaskStatus::VersionTooLow, "1.0.1");
    QVERIFY(ui_cli->validationLabel->text().contains("too low"));
    QVERIFY(GET_BUTTON(NextButton)->isEnabled() == false);
    QVERIFY(ui_cli->detailsWidget->isVisible() == true);
    VISUAL_WAIT;

    // Ensure that the wizard is not running when we quit
    clean_quit(wizard);
}

QTEST_MAIN(TestSetupWizard)
WARNINGS_DISABLE
#include "test-setupwizard.moc"
WARNINGS_ENABLE

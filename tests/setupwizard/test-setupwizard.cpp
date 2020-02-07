#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QSettings>

#include "ui_setupdialog.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "utils.h"

#include <TSettings.h>

#include "compare-settings.h"

#include "setupdialog.h"

#define GET_UI_PAGE(x, PAGETYPE)                                               \
    do                                                                         \
    {                                                                          \
        x = wizard->get_ui();                                                  \
    } while(0)

#define GET_BUTTON(x) wizard->button(SetupDialog::x)

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
    void helper_almost_normal_install(SetupDialog *wizard);
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
    QCoreApplication::processEvents(QEventLoop::AllEvents, 5000);
}

void TestSetupWizard::do_nothing()
{
    SetupDialog *wizard = new SetupDialog();

    // This test is intended to help debug memory leaks.
    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

    delete wizard;
}

void TestSetupWizard::helper_almost_normal_install(SetupDialog *wizard)
{
    QSignalSpy sig_cli(wizard, SIGNAL(tarsnapVersionRequested()));
    QSignalSpy sig_register(wizard,
                            SIGNAL(registerMachineRequested(QString, bool)));

    Ui::SetupDialog *ui_cli;
    Ui::SetupDialog *ui_register;

    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

    // Page 1
    QVERIFY(wizard->pageTitle() == "Setup wizard");
    GET_BUTTON(NextButton)->click();
    VISUAL_WAIT;

    // Page 2
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    GET_UI_PAGE(ui_cli, CliPage);
    VISUAL_WAIT;

    // Fake the CLI detection and checking
    QVERIFY(sig_cli.count() == 1);
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(ui_cli->cliValidationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    // Proceed
    GET_BUTTON(NextButton)->click();
    VISUAL_WAIT;

    // Page 3
    QVERIFY(wizard->pageTitle() == "Register with server");
    GET_UI_PAGE(ui_register, RegisterPage);

    // Pretend that we already have a key
    ui_register->useExistingKeyfileButton->click();
    ui_register->machineKeyCombo->setCurrentText("empty.key");
    VISUAL_WAIT;

    // Pretend to register
    GET_BUTTON(NextButton)->click();
    QVERIFY(sig_register.count() == 1);
    wizard->updateLoadingAnimation(false);
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

    SetupDialog *wizard = new SetupDialog();

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
    delete wizard;
}

void TestSetupWizard::cancel_install()
{
    TARSNAP_CLI_OR_SKIP;

    SetupDialog *wizard = new SetupDialog();

    // Almost complete a normal install
    helper_almost_normal_install(wizard);

    // Close before we actually finish
    QTest::keyEvent(QTest::Click, wizard, Qt::Key_Escape);
    VISUAL_WAIT;

    // Clean up
    delete wizard;

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

    SetupDialog *wizard = new SetupDialog();

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
    delete wizard;
}

void TestSetupWizard::cli()
{
    SetupDialog *    wizard = new SetupDialog();
    Ui::SetupDialog *ui_cli;

    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

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
    QVERIFY(ui_cli->cliValidationLabel->text()
            == "Invalid App data directory set.");
    VISUAL_WAIT;
    ui_cli->appdataPathLineBrowse->setText("/tmp");

    // Cache directory
    ui_cli->cachePathLineBrowse->setText("fake-dir");
    QVERIFY(ui_cli->cliValidationLabel->text()
            == "Invalid Tarsnap cache directory set.");
    VISUAL_WAIT;
    ui_cli->cachePathLineBrowse->setText("/tmp");

    // Tarsnap CLI directory
    ui_cli->cliPathLineBrowse->setText("fake-dir");
    QVERIFY(ui_cli->cliValidationLabel->text().contains(
        "Tarsnap utilities not found."));
    VISUAL_WAIT;
    ui_cli->cliPathLineBrowse->setText("/tmp");

    // Fake detecting the binaries
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(ui_cli->cliValidationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    delete wizard;
}

void TestSetupWizard::version_too_low()
{
    SetupDialog *    wizard = new SetupDialog();
    Ui::SetupDialog *ui_cli;

    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

    // Advance to CLI page
    GET_BUTTON(NextButton)->click();
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    GET_UI_PAGE(ui_cli, CliPage);
    VISUAL_WAIT;

    // Fake detecting the binaries with a too-low version number
    wizard->tarsnapVersionResponse(TaskStatus::VersionTooLow, "1.0.1");
    QVERIFY(ui_cli->cliValidationLabel->text().contains("too low"));
    QVERIFY(GET_BUTTON(NextButton)->isEnabled() == false);
    QVERIFY(ui_cli->cliAdvancedWidget->isVisible() == true);
    VISUAL_WAIT;

    delete wizard;
}

QTEST_MAIN(TestSetupWizard)
#include "test-setupwizard.moc"

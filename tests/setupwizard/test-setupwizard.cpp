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

class TestSetupWizard : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void normal_install();
    void cancel_install();
    void skip_install();
    void cli();
    void version_too_low();

private:
    void helper_almost_normal_install(SetupDialog *wizard);
};

void TestSetupWizard::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }
}

void TestSetupWizard::cleanupTestCase()
{
    TSettings::destroy();
}

void TestSetupWizard::helper_almost_normal_install(SetupDialog *wizard)
{
    Ui::SetupDialog *ui = wizard->_ui;
    QSignalSpy       sig_cli(wizard, SIGNAL(tarsnapVersionRequested()));
    QSignalSpy       sig_register(wizard,
                            SIGNAL(registerMachineRequested(QString, bool)));

    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

    // Page 1
    QVERIFY(wizard->pageTitle() == "Setup wizard");
    wizard->next();
    VISUAL_WAIT;

    // Page 2
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    VISUAL_WAIT;

    // Fake the CLI detection and checking
    QVERIFY(sig_cli.count() == 1);
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(ui->cliValidationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    // Proceed
    wizard->next();
    VISUAL_WAIT;

    // Page 3
    QVERIFY(wizard->pageTitle() == "Register with server");
    // Pretend that we already have a key
    ui->useExistingKeyfileButton->click();
    ui->machineKeyCombo->setCurrentText("empty.key");
    VISUAL_WAIT;

    // Pretend to register
    wizard->next();
    QVERIFY(sig_register.count() == 1);
    VISUAL_WAIT;

    // Fake a response
    wizard->registerMachineResponse(TaskStatus::Completed, "");
    VISUAL_WAIT;

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
    wizard->next();
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

    SetupDialog *    wizard = new SetupDialog();
    Ui::SetupDialog *ui     = wizard->_ui;

    // Almost complete a normal install
    helper_almost_normal_install(wizard);

    // Now go back to the beginning and skip the install
    wizard->back();
    wizard->back();
    wizard->back();
    VISUAL_WAIT;

    QTest::mouseClick(ui->backButton, Qt::LeftButton);
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
    Ui::SetupDialog *ui     = wizard->_ui;

    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

    // Advance to CLI page and expand advanced options
    wizard->next();
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    VISUAL_WAIT;

    // We may or may not receive a version query, depending on whether
    // tarsnap is installed, so don't check for this here.
    // (We check "did we receive a version-query signal" in other
    // tarsnap-dependent tests.)
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    VISUAL_WAIT;

    // App data directory
    SET_TEXT_WITH_SIGNAL(ui->appDataPathLineEdit, "fake-dir");
    QVERIFY(ui->cliValidationLabel->text()
            == "Invalid App data directory set.");
    VISUAL_WAIT;
    SET_TEXT_WITH_SIGNAL(ui->appDataPathLineEdit, "/tmp");

    // Cache directory
    SET_TEXT_WITH_SIGNAL(ui->tarsnapCacheLineEdit, "fake-dir");
    QVERIFY(ui->cliValidationLabel->text()
            == "Invalid Tarsnap cache directory set.");
    VISUAL_WAIT;
    SET_TEXT_WITH_SIGNAL(ui->tarsnapCacheLineEdit, "/tmp");

    // Tarsnap CLI directory
    SET_TEXT_WITH_SIGNAL(ui->tarsnapPathLineEdit, "fake-dir");
    QVERIFY(ui->cliValidationLabel->text().contains(
        "Tarsnap utilities not found."));
    VISUAL_WAIT;
    SET_TEXT_WITH_SIGNAL(ui->tarsnapPathLineEdit, "/tmp");

    // Fake detecting the binaries
    wizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(ui->cliValidationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    delete wizard;
}

void TestSetupWizard::version_too_low()
{
    SetupDialog *    wizard = new SetupDialog();
    Ui::SetupDialog *ui     = wizard->_ui;

    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

    // Advance to CLI page
    wizard->next();
    QVERIFY(wizard->pageTitle() == "Command-line utilities");
    VISUAL_WAIT;

    // Fake detecting the binaries with a too-low version number
    wizard->tarsnapVersionResponse(TaskStatus::VersionTooLow, "1.0.1");
    QVERIFY(ui->cliValidationLabel->text().contains("too low"));
    QVERIFY(ui->nextButton->isEnabled() == false);
    QVERIFY(ui->cliAdvancedWidget->isVisible() == true);
    VISUAL_WAIT;

    delete wizard;
}

QTEST_MAIN(TestSetupWizard)
#include "test-setupwizard.moc"

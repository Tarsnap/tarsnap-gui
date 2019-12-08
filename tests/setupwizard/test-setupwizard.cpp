#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QSettings>

#include "ui_setupdialog.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "utils.h"

#include <TSettings.h>

#include "setupdialog.h"

//! Compares all the keys in two QSettings files.
static bool compareSettings(QSettings *settings, QSettings *target)
{
    // On OSX, QSettings returns a bunch of values for the system
    // itself (e.g., "com/apple/trackpad/enableSecondaryClick",
    // NSNavRecentPlaces").  To avoid this, we only examine the
    // groups that are present in the target config file.
    for(int g = 0; g < target->childGroups().length(); g++)
    {
        QString group = target->childGroups().at(g);

        settings->beginGroup(group);
        target->beginGroup(group);

        // Check length of key list
        QStringList settings_keys = settings->allKeys();
        if(settings_keys.length() != target->allKeys().length())
        {
            qDebug() << "compareSettings: number of keys does not match!"
                     << settings_keys.length() << target->allKeys().length();
            return false;
        }

        // Check each key's value
        for(int i = 0; i < settings_keys.length(); i++)
        {
            QString key = settings_keys.at(i);

            // Skip over keys that will be different
            if((group == "tarsnap") && (key == "machine"))
                continue;
#ifdef Q_OS_OSX
            if((group == "tarsnap") && (key == "cache"))
                continue;
            if((group == "app") && (key == "app_data"))
                continue;
#endif

            // Skip over key(s) that can plausibly be different
            if((group == "tarsnap") && (key == "path"))
                continue;

            // Compare values
            if(settings->value(key) != target->value(key))
            {
                qDebug() << "compareSettings: values do not match!" << key
                         << settings->value(key) << target->value(key);
                return false;
            }
        }
        settings->endGroup();
        target->endGroup();
    }

    return true;
}

class TestSetupWizard : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void cancel_install();
    void normal_install();
    void cli();
    void version_too_low();
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

void TestSetupWizard::cancel_install()
{
    TARSNAP_CLI_OR_SKIP;

    SetupDialog *    setupWizard = new SetupDialog();
    Ui::SetupDialog *ui          = setupWizard->_ui;

    VISUAL_INIT(setupWizard);

    // Page 1
    QVERIFY(ui->titleLabel->text() == "Setup wizard");
    QTest::mouseClick(ui->backButton, Qt::LeftButton);
    VISUAL_WAIT;

    // Clean up
    TSettings::destroy();
    delete setupWizard;
}

void TestSetupWizard::normal_install()
{
    TARSNAP_CLI_OR_SKIP;

    SetupDialog *    setupWizard = new SetupDialog();
    Ui::SetupDialog *ui          = setupWizard->_ui;
    QSignalSpy       sig_cli(setupWizard, SIGNAL(tarsnapVersionRequested()));
    QSignalSpy       sig_register(setupWizard,
                            SIGNAL(registerMachineRequested(QString, bool)));

    VISUAL_INIT(setupWizard);

    // Page 1
    QVERIFY(ui->titleLabel->text() == "Setup wizard");
    QTest::mouseClick(ui->nextButton, Qt::LeftButton);
    VISUAL_WAIT;

    // Page 2
    QVERIFY(ui->titleLabel->text() == "Command-line utilities");
    QVERIFY(sig_cli.count() == 1);
    // Fake the CLI detection and checking
    setupWizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(
        ui->advancedValidationLabel->text().contains("Tarsnap CLI version"));
    QTest::mouseClick(ui->nextButton, Qt::LeftButton);
    VISUAL_WAIT;

    // Page 3
    QVERIFY(ui->titleLabel->text() == "Register with server");
    // Pretend that we already have a key
    setupWizard->restoreYes();
    ui->restoreYesButton->setChecked(true);
    ui->machineKeyCombo->setCurrentText("fake.key");
    ui->nextButton->setEnabled(true);
    QTest::mouseClick(ui->nextButton, Qt::LeftButton);
    // Check results of registration
    QVERIFY(sig_register.count() == 1);
    setupWizard->registerMachineResponse(TaskStatus::Completed, "");
    VISUAL_WAIT;

    // Page 4
    QVERIFY(ui->titleLabel->text() == "Setup complete!");
    QTest::mouseClick(ui->nextButton, Qt::LeftButton);
    VISUAL_WAIT;

    // Check resulting init file.  The first can be in any format (for now).
    TSettings settings;
    QSettings target("after-test.conf", QSettings::IniFormat);
    QVERIFY(compareSettings(settings.getQSettings(), &target));

    // Clean up
    TSettings::destroy();
    delete setupWizard;
}

void TestSetupWizard::cli()
{
    SetupDialog *    setupWizard = new SetupDialog();
    Ui::SetupDialog *ui          = setupWizard->_ui;
    QSignalSpy       sig_cli(setupWizard, SIGNAL(tarsnapVersionRequested()));

    VISUAL_INIT(setupWizard);

    // Advance to CLI page and expand advanced options
    QTest::mouseClick(ui->nextButton, Qt::LeftButton);
    QVERIFY(ui->titleLabel->text() == "Command-line utilities");
    ui->advancedCLIButton->click();
    VISUAL_WAIT;

    // App data directory
    SET_TEXT_WITH_SIGNAL(ui->appDataPathLineEdit, "fake-dir");
    QVERIFY(ui->advancedValidationLabel->text()
            == "Invalid App data directory set.");
    VISUAL_WAIT;
    SET_TEXT_WITH_SIGNAL(ui->appDataPathLineEdit, "/tmp");

    // Cache directory
    SET_TEXT_WITH_SIGNAL(ui->tarsnapCacheLineEdit, "fake-dir");
    QVERIFY(ui->advancedValidationLabel->text()
            == "Invalid Tarsnap cache directory set.");
    VISUAL_WAIT;
    SET_TEXT_WITH_SIGNAL(ui->tarsnapCacheLineEdit, "/tmp");

    // Tarsnap CLI directory
    SET_TEXT_WITH_SIGNAL(ui->tarsnapPathLineEdit, "fake-dir");
    QVERIFY(ui->advancedValidationLabel->text().contains(
        "Tarsnap utilities not found."));
    VISUAL_WAIT;
    SET_TEXT_WITH_SIGNAL(ui->tarsnapPathLineEdit, "/tmp");

    // Fake detecting the binaries
    setupWizard->tarsnapVersionResponse(TaskStatus::Completed, "X.Y.Z");
    QVERIFY(
        ui->advancedValidationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    delete setupWizard;
}

void TestSetupWizard::version_too_low()
{
    SetupDialog *    setupWizard = new SetupDialog();
    Ui::SetupDialog *ui          = setupWizard->_ui;

    VISUAL_INIT(setupWizard);

    // Advance to CLI page
    QTest::mouseClick(ui->nextButton, Qt::LeftButton);
    QVERIFY(ui->titleLabel->text() == "Command-line utilities");
    VISUAL_WAIT;

    // Fake detecting the binaries with a too-low version number
    setupWizard->tarsnapVersionResponse(TaskStatus::VersionTooLow, "1.0.1");
    QVERIFY(ui->advancedValidationLabel->text().contains("too low"));
    QVERIFY(ui->nextButton->isEnabled() == false);
    VISUAL_WAIT;

    delete setupWizard;
}

QTEST_MAIN(TestSetupWizard)
#include "test-setupwizard.moc"

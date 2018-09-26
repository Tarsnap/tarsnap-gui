#include <QSettings>
#include <QtTest/QtTest>

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
    void normal_install();
    void cli();
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

void TestSetupWizard::normal_install()
{
    TARSNAP_CLI_OR_SKIP;

    SetupDialog *   setupWizard = new SetupDialog();
    Ui::SetupDialog ui          = setupWizard->_ui;
    QSignalSpy      sig_cli(setupWizard, SIGNAL(getTarsnapVersion(QString)));
    QSignalSpy      sig_register(setupWizard,
                            SIGNAL(requestRegisterMachine(QString, QString,
                                                          QString, QString,
                                                          QString, QString)));

    VISUAL_INIT(setupWizard);

    // Page 1
    QVERIFY(ui.titleLabel->text() == "Setup wizard");
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);
    VISUAL_WAIT;

    // Page 2
    QVERIFY(ui.titleLabel->text() == "Command-line utilities");
    QVERIFY(sig_cli.count() == 1);
    // Fake the CLI detection and checking
    setupWizard->setTarsnapVersion("X.Y.Z");
    QVERIFY(ui.advancedValidationLabel->text().contains("Tarsnap CLI version"));
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);
    VISUAL_WAIT;

    // Page 3
    QVERIFY(ui.titleLabel->text() == "Register with server");
    // Pretend that we already have a key
    setupWizard->restoreYes();
    ui.restoreYesButton->setChecked(true);
    ui.machineKeyCombo->setCurrentText("fake.key");
    ui.nextButton->setEnabled(true);
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);
    // Check results of registration
    QVERIFY(sig_register.count() == 1);
    QVERIFY(sig_register.takeFirst().at(3).toString() == QString("fake.key"));
    setupWizard->registerMachineStatus(TaskStatus::Completed, "");
    VISUAL_WAIT;

    // Page 4
    QVERIFY(ui.titleLabel->text() == "Setup complete!");
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);
    VISUAL_WAIT;

    // Check resulting init file.  The first can be in any format (for now).
    TSettings settings;
    QSettings target("after-test.conf", QSettings::IniFormat);
    QVERIFY(compareSettings(settings.getQSettings(), &target));

    delete setupWizard;
}

void TestSetupWizard::cli()
{
    SetupDialog *   setupWizard = new SetupDialog();
    Ui::SetupDialog ui          = setupWizard->_ui;
    QSignalSpy      sig_cli(setupWizard, SIGNAL(getTarsnapVersion(QString)));

    VISUAL_INIT(setupWizard);

    // Advanced to CLI page and expand advanced options
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);
    QVERIFY(ui.titleLabel->text() == "Command-line utilities");
    ui.advancedCLIButton->click();
    VISUAL_WAIT;

    // App data directory
    ui.appDataPathLineEdit->setText("fake-dir");
    QVERIFY(ui.advancedValidationLabel->text()
            == "Invalid App data directory set.");
    VISUAL_WAIT;
    ui.appDataPathLineEdit->setText("/tmp");

    // Cache directory
    ui.tarsnapCacheLineEdit->setText("fake-dir");
    QVERIFY(ui.advancedValidationLabel->text()
            == "Invalid Tarsnap cache directory set.");
    VISUAL_WAIT;
    ui.tarsnapCacheLineEdit->setText("/tmp");

    // Tarsnap CLI directory
    ui.tarsnapPathLineEdit->setText("fake-dir");
    QVERIFY(ui.advancedValidationLabel->text().contains(
        "Tarsnap utilities not found."));
    VISUAL_WAIT;
    ui.tarsnapPathLineEdit->setText("/tmp");

    // Fake detecting the binaries
    setupWizard->setTarsnapVersion("X.Y.Z.");
    QVERIFY(ui.advancedValidationLabel->text().contains("Tarsnap CLI version"));
    VISUAL_WAIT;

    delete setupWizard;
}

QTEST_MAIN(TestSetupWizard)
#include "test-setupwizard.moc"

#include <QtTest/QtTest>

#include "../qtest-platform.h"
#include "utils.h"

#include "setupdialog.h"

#define VISUAL_INIT                                                            \
    IF_VISUAL                                                                  \
    {                                                                          \
        setupWizard->show();                                                   \
        VISUAL_WAIT;                                                           \
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
    SetupDialog *   setupWizard = new SetupDialog();
    Ui::SetupDialog ui          = setupWizard->_ui;
    QSignalSpy      sig_cli(setupWizard, SIGNAL(getTarsnapVersion(QString)));
    QSignalSpy      sig_register(setupWizard,
                            SIGNAL(requestRegisterMachine(QString, QString,
                                                          QString, QString,
                                                          QString, QString)));

    VISUAL_INIT;

    // If there's no tarsnap binary, skip this test.
    if(Utils::findTarsnapClientInPath(QString(""), false).isEmpty())
    {
        delete setupWizard;
        QSKIP("No tarsnap binary found");
    }

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
    ui.machineKeyCombo->setCurrentText("pretend.key");
    ui.nextButton->setEnabled(true);
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);
    // Check results of registration
    QVERIFY(sig_register.count() == 1);
    QVERIFY(sig_register.takeFirst().at(3).toString()
            == QString("pretend.key"));
    setupWizard->registerMachineStatus(TaskStatus::Completed, "");
    VISUAL_WAIT;

    // Page 4
    QVERIFY(ui.titleLabel->text() == "Setup complete!");
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);
    VISUAL_WAIT;

    delete setupWizard;
}

void TestSetupWizard::cli()
{
    SetupDialog *   setupWizard = new SetupDialog();
    Ui::SetupDialog ui          = setupWizard->_ui;
    QSignalSpy      sig_cli(setupWizard, SIGNAL(getTarsnapVersion(QString)));

    VISUAL_INIT;

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

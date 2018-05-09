#include <QtTest/QtTest>

#include "setupdialog.h"

class TestSetupWizard : public QObject
{
    Q_OBJECT

public:
    TestSetupWizard();
    ~TestSetupWizard();

private slots:
    void normal_install();

private:
};

TestSetupWizard::TestSetupWizard()
{
    QCoreApplication::setOrganizationName("test-config");
}

TestSetupWizard::~TestSetupWizard()
{
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

    // Page 1
    QVERIFY(ui.titleLabel->text() == "Setup wizard");
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);

    // Page 2
    QVERIFY(ui.titleLabel->text() == "Command-line utilities");
    QVERIFY(sig_cli.count() == 1);
    // Fake the CLI detection and checking
    setupWizard->setTarsnapVersion("X.Y.Z");
    QVERIFY(ui.advancedValidationLabel->text().contains("Tarsnap CLI version"));
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);

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

    // Page 4
    QVERIFY(ui.titleLabel->text() == "Setup complete!");
    QTest::mouseClick(ui.nextButton, Qt::LeftButton);

    delete setupWizard;
}

QTEST_MAIN(TestSetupWizard)
#include "testsetupwizard.moc"

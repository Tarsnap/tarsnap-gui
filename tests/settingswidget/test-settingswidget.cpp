#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCheckBox>
#include <QCoreApplication>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QToolBox>
#include <QVariant>

#include "ui_settingswidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "tarsnapaccount.h"
#include "translator.h"
#include "widgets/confirmationdialog.h"
#include "widgets/tarsnapaccountdialog.h"

#include "ConsoleLog.h"
#include "TSettings.h"

#include "widgets/settingswidget.h"

class TestSettingsWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void tarsnapAccount();
    void tarsnapAccountDialog();
    void account();
    void backup();
    void application();
};

void TestSettingsWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    HANDLE_IGNORING_XDG_HOME;

    ConsoleLog::initializeConsoleLog();
    Translator::initializeTranslator();

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }
}

void TestSettingsWidget::cleanupTestCase()
{
    Translator::destroy();
    TSettings::destroy();
    ConsoleLog::destroy();

    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

void TestSettingsWidget::tarsnapAccount()
{
    TarsnapAccount *ta = new TarsnapAccount();

    delete ta;
}

void TestSettingsWidget::tarsnapAccountDialog()
{
    TarsnapAccountDialog *ta = new TarsnapAccountDialog();

    // Trigger an error message that we have to click away.
    ta->getAccountInfo();
    VISUAL_WAIT;
    ta->close();
    QTest::qWait(200);

    delete ta;
}

void TestSettingsWidget::account()
{
    SettingsWidget     *settingsWidget = new SettingsWidget();
    Ui::SettingsWidget *ui             = settingsWidget->_ui;
    QSignalSpy          sig_status(settingsWidget,
                                   SIGNAL(newStatusMessage(QString, QString)));

    TarsnapAccountDialog *tarsnapAccountDialog =
        settingsWidget->_tarsnapAccountDialog;

    VISUAL_INIT(settingsWidget);

    // Send stats info.
    settingsWidget->setArchiveCount(1);

    // Trigger an error message that we have to click away.
    ui->updateAccountButton->clicked();
    VISUAL_WAIT;
    tarsnapAccountDialog->_popup->close();
    VISUAL_WAIT;

    // Set username, machine name, key.
    SET_TEXT_WITH_SIGNAL(ui->accountUserLineEdit, "edited-user");
    SET_TEXT_WITH_SIGNAL(ui->accountMachineKeyLineEdit, "edited-mk");
    VISUAL_WAIT;

    // Nuke button, reject dialog.
    ui->nukeArchivesButton->clicked();
    VISUAL_WAIT;
    settingsWidget->_nukeConfirmationDialog->_inputDialog->reject();
    QTest::qWait(200);
    QVERIFY(sig_status.takeFirst().at(0).toString()
            == QString("Nuke confirmation requested."));
    QVERIFY(sig_status.takeFirst().at(0).toString()
            == QString("Nuke cancelled."));
    VISUAL_WAIT;

    // Check saved settings
    TSettings settings;

    QVERIFY(settings.value("tarsnap/user", "") == QString("edited-user"));
    QVERIFY(settings.value("tarsnap/key", "") == QString("edited-mk"));

    delete settingsWidget;
}

void TestSettingsWidget::backup()
{
    SettingsWidget     *settingsWidget = new SettingsWidget();
    Ui::SettingsWidget *ui             = settingsWidget->_ui;

    VISUAL_INIT(settingsWidget);

    // Switch to Backup tab.
    ui->settingsToolbox->setCurrentIndex(1);
    VISUAL_WAIT;

    // Toggle some options.
    ui->preservePathsCheckBox->setChecked(false);
    ui->skipNoDumpCheckBox->setChecked(true);
    ui->simulationCheckBox->setChecked(true);
    VISUAL_WAIT;

    // Check saved settings.  These are ready due to not using setText().
    TSettings settings;

    QVERIFY(settings.value("tarsnap/preserve_pathnames", "").toBool() == false);
    QVERIFY(settings.value("app/skip_nodump", "").toBool() == true);
    QVERIFY(settings.value("tarsnap/dry_run", "").toBool() == true);

    delete settingsWidget;
}

void TestSettingsWidget::application()
{
    SettingsWidget     *settingsWidget = new SettingsWidget();
    Ui::SettingsWidget *ui             = settingsWidget->_ui;

    VISUAL_INIT(settingsWidget);

    // Switch to Application tab.
    ui->settingsToolbox->setCurrentIndex(2);
    VISUAL_WAIT;

    // Toggle some options.
    ui->notificationsCheckBox->setChecked(false);
    VISUAL_WAIT;

    // Check saved settings.  These are ready due to not using setText().
    TSettings settings;

    QVERIFY(settings.value("app/notifications", "").toBool() == false);

    delete settingsWidget;
}

QTEST_MAIN(TestSettingsWidget)
WARNINGS_DISABLE
#include "test-settingswidget.moc"
WARNINGS_ENABLE

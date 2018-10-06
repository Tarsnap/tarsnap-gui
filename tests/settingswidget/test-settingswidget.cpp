#include <QtTest/QtTest>

#include "../qtest-platform.h"
#include "utils.h"

#include "confirmationdialog.h"

#include <TSettings.h>

#include "settingswidget.h"

class TestSettingsWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void account();
    void backup();
    void application();
};

void TestSettingsWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }
}

void TestSettingsWidget::cleanupTestCase()
{
    TSettings::destroy();
}

void TestSettingsWidget::account()
{
    HANDLE_IGNORING_XDG_HOME;

    SettingsWidget *   settingsWidget = new SettingsWidget();
    Ui::SettingsWidget ui             = settingsWidget->_ui;
    TarsnapAccount *   tarsnapAccount = &settingsWidget->_tarsnapAccount;
    QSignalSpy         sig_status(settingsWidget,
                          SIGNAL(newStatusMessage(QString, QString)));

    VISUAL_INIT(settingsWidget);

    // Send stats info.
    settingsWidget->overallStatsChanged(5, 4, 3, 2, 1);
    QCOMPARE(ui.accountArchivesCountLabel->text(), QString("1"));
    QCOMPARE(ui.accountTotalSizeLabel->text(), QString("5 B"));
    QCOMPARE(ui.accountActualSizeLabel->text(), QString("2 B"));
    QCOMPARE(ui.accountStorageSavedLabel->text(), QString("3 B"));
    VISUAL_WAIT;

    // Trigger an error message that we have to click away.
    QMetaObject::invokeMethod(ui.updateAccountButton, "clicked",
                              Qt::QueuedConnection);
    QMetaObject::invokeMethod(&tarsnapAccount->_popup, "close",
                              Qt::QueuedConnection);
    VISUAL_WAIT;

    // Set username, machine name, key.
    ui.accountUserLineEdit->setText("edited-user");
    ui.accountMachineLineEdit->setText("edited-mn");
    ui.accountMachineKeyLineEdit->setText("edited-mk");
    VISUAL_WAIT;

    // Nuke button, reject dialog.
    QMetaObject::invokeMethod(ui.nukeArchivesButton, "clicked",
                              Qt::QueuedConnection);
    QMetaObject::invokeMethod(&settingsWidget->_nukeConfirmationDialog._inputDialog,
                              "reject", Qt::QueuedConnection);
    QTest::qWait(200);
    QVERIFY(sig_status.takeFirst().at(0).toString()
            == QString("Nuke confirmation requested."));
    QVERIFY(sig_status.takeFirst().at(0).toString()
            == QString("Nuke cancelled."));
    VISUAL_WAIT;

    // It would be nice to test the nuke button with invalid confirmation text,
    // but QInputDialog::setTextValue() is not a slot and ::invokeMethod()
    // only works on slots.

    // Call commitSettings() manually because setText() doesn't trigger
    // editingFinished(), which is what's attached to the signal.
    settingsWidget->commitSettings();

    // Check saved settings
    TSettings settings;

    QVERIFY(settings.value("tarsnap/user", "") == QString("edited-user"));
    QVERIFY(settings.value("tarsnap/machine", "") == QString("edited-mn"));
    QVERIFY(settings.value("tarsnap/key", "") == QString("edited-mk"));

    delete settingsWidget;
}

void TestSettingsWidget::backup()
{
    SettingsWidget *   settingsWidget = new SettingsWidget();
    Ui::SettingsWidget ui             = settingsWidget->_ui;

    VISUAL_INIT(settingsWidget);

    // Switch to Backup tab.
    ui.settingsToolbox->setCurrentIndex(1);
    VISUAL_WAIT;

    // Toggle some options.
    ui.preservePathsCheckBox->setChecked(false);
    ui.skipNoDumpCheckBox->setChecked(true);
    ui.simulationCheckBox->setChecked(true);
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
    SettingsWidget *   settingsWidget = new SettingsWidget();
    Ui::SettingsWidget ui             = settingsWidget->_ui;

    VISUAL_INIT(settingsWidget);

    // Switch to Application tab.
    ui.settingsToolbox->setCurrentIndex(2);
    VISUAL_WAIT;

    // Toggle some options.
    ui.notificationsCheckBox->setChecked(false);
    VISUAL_WAIT;

    // Check saved settings.  These are ready due to not using setText().
    TSettings settings;

    QVERIFY(settings.value("app/notifications", "").toBool() == false);

    delete settingsWidget;
}

QTEST_MAIN(TestSettingsWidget)
#include "test-settingswidget.moc"

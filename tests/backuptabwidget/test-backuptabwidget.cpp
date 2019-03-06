#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include "ui_backuptabwidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "backuplistwidget.h"
#include "backuptabwidget.h"

#include "TSettings.h"

class TestBackupTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void backupListWidget();
    void nameEdit();
};

void TestBackupTabWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }
}

void TestBackupTabWidget::cleanupTestCase()
{
    TSettings::destroy();
}

void TestBackupTabWidget::backupListWidget()
{
    BackupListWidget *blw = new BackupListWidget();
    delete blw;
}

void TestBackupTabWidget::nameEdit()
{
    BackupTabWidget *    backuptabwidget = new BackupTabWidget();
    Ui::BackupTabWidget *ui              = backuptabwidget->_ui;

    VISUAL_INIT(backuptabwidget);

    // Start off without anything in the LineEdit
    QVERIFY(ui->backupNameLineEdit->text().isEmpty());
    VISUAL_WAIT;

    // Append the timestamp
    ui->appendTimestampCheckBox->toggle();
    QVERIFY(ui->backupNameLineEdit->text().isEmpty() == false);
    VISUAL_WAIT;

    // Remove the timestamp
    ui->appendTimestampCheckBox->toggle();
    QVERIFY(ui->backupNameLineEdit->text().isEmpty());
    VISUAL_WAIT;

    delete backuptabwidget;
}

QTEST_MAIN(TestBackupTabWidget)
#include "test-backuptabwidget.moc"

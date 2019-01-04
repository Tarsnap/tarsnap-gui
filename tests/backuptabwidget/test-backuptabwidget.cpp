#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "backuptabwidget.h"

#include "ui_backuptabwidget.h"

#include "TSettings.h"

class TestBackupTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

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

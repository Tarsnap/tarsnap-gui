#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "backuptabwidget.h"

class TestBackupTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
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

QTEST_MAIN(TestBackupTabWidget)
#include "test-backuptabwidget.moc"

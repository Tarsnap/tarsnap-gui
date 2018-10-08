#include <QtTest/QtTest>

#include "../qtest-platform.h"
#include "utils.h"

#include <TSettings.h>

#include "jobstabwidget.h"

class TestJobsTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
};

void TestJobsTabWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }
}

void TestJobsTabWidget::cleanupTestCase()
{
    TSettings::destroy();
}

QTEST_MAIN(TestJobsTabWidget)
#include "test-jobstabwidget.moc"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>
WARNINGS_ENABLE

#include "debug.h"

#include <TSettings.h>

class TestConsoleLog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void saveMessage();
};

void TestConsoleLog::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    LOG.initializeConsoleLog();
}

void TestConsoleLog::cleanupTestCase()
{
    TSettings::destroy();
    ConsoleLog::destroy();
}

void TestConsoleLog::saveMessage()
{
    TSettings settings;

    // Don't record this message
    LOG << "don't write this";

    // Save a message
    settings.setValue("app/save_console_log", true);
    LOG << "write this";

    // Disable saving again
    settings.setValue("app/save_console_log", false);
    LOG << "don't write this";
}

QTEST_MAIN(TestConsoleLog)
#include "test-consolelog.moc"

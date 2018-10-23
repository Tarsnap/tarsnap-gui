#include <QtTest/QtTest>

#include "debug.h"

#include <TSettings.h>

class TestConsoleLog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void basic();
    void initializeConsoleLog();
    void saveMessage();
};

void TestConsoleLog::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);
}

void TestConsoleLog::cleanupTestCase()
{
    TSettings::destroy();
}

void TestConsoleLog::basic()
{
    ConsoleLog &log = ConsoleLog::instance();
    (void)log;
}

void TestConsoleLog::initializeConsoleLog()
{
    ConsoleLog::instance().initializeConsoleLog();
}

void TestConsoleLog::saveMessage()
{
    ConsoleLog &log = ConsoleLog::instance();
    TSettings   settings;

    // Don't record this message
    log << "don't write this";

    // Save a message
    settings.setValue("app/save_console_log", true);
    log << "write this";

    // Disable saving again
    settings.setValue("app/save_console_log", false);
    log << "don't write this";
}

QTEST_MAIN(TestConsoleLog)
#include "test-consolelog.moc"

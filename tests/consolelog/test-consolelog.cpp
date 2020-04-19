#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDir>
#include <QObject>
#include <QString>
#include <QTest>
#include <QVariant>
WARNINGS_ENABLE

#include "ConsoleLog.h"
#include "TSettings.h"

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

    QString appdata = settings.value("app/app_data", "").toString();
    QString logFile = appdata + QDir::separator() + TEST_NAME + ".log";

    // Don't record this message
    LOG << "don't write this\n";

    // Save a message
    LOG.setFilename(logFile);
    LOG.setWriteToFile(true);
    LOG << "write this\n";

    // Disable saving again
    LOG.setWriteToFile(false);
    LOG << "don't write this\n";
}

QTEST_MAIN(TestConsoleLog)
WARNINGS_DISABLE
#include "test-consolelog.moc"
WARNINGS_ENABLE

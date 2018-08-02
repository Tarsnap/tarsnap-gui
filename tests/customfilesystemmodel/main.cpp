#include <QtTest/QtTest>

#include "testCFSM.h"

#include "scenario-num.h"

class TestTask : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void runScenario();
    void runScenario_data();

private:
    TestCFSM *tester;
};

void TestTask::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    tester = new TestCFSM();
}

void TestTask::cleanupTestCase()
{
    delete tester;
}

void TestTask::runScenario()
{
    QFETCH(int, scenario_number);
    QVERIFY(tester->runScenario(scenario_number) == 0);
}

void TestTask::runScenario_data()
{
    QTest::addColumn<int>("scenario_number");

    for(int i = 0; i < NUM_SCENARIOS; i++)
    {
        QString scenarioFilename =
            QString("scenario-%1.txt").arg(i, 2, 10, QChar('0'));
        QTest::newRow(scenarioFilename.toLatin1()) << i;
    }
}

QTEST_MAIN(TestTask)
#include "main.moc"

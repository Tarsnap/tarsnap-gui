#include <QtTest/QtTest>

#include "run-scenario.h"
#include "scenario-num.h"

class TestCFSM : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void runScenario();
    void runScenario_data();
};

void TestCFSM::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);
}

void TestCFSM::runScenario()
{
    QFETCH(int, scenario_number);

    RunScenario *runner;
    runner = new RunScenario();

    QVERIFY(runner->runScenario(scenario_number) == 0);

    delete runner;
}

void TestCFSM::runScenario_data()
{
    QTest::addColumn<int>("scenario_number");

    for(int i = 0; i < NUM_SCENARIOS; i++)
    {
        QString scenarioFilename =
            QString("scenario-%1.txt").arg(i, 2, 10, QChar('0'));
        QTest::newRow(scenarioFilename.toLatin1()) << i;
    }
}

QTEST_MAIN(TestCFSM)
#include "test-cfsm.moc"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QByteArray>
#include <QChar>
#include <QCoreApplication>
#include <QList>
#include <QObject>
#include <QPersistentModelIndex>
#include <QString>
#include <QTest>
#include <QTestData>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include "run-scenario.h"
#include "scenario-num.h"

#include "customfilesystemmodel.h"

class TestCFSM : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void setGet();
    void runScenario();
    void runScenario_data();
};

void TestCFSM::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);
}

void TestCFSM::setGet()
{
    CustomFileSystemModel *model = new CustomFileSystemModel();

    // Start loading the test dir
    model->setData(model->index(TEST_DIR), Qt::Checked, Qt::CheckStateRole);

    // We should get the test dir back
    QList<QPersistentModelIndex> list     = model->checkedIndexes();
    QString                      filename = list.at(0).data().toString();
    QVERIFY(filename == TEST_NAME);

    delete model;
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
WARNINGS_DISABLE
#include "test-cfsm.moc"
WARNINGS_ENABLE

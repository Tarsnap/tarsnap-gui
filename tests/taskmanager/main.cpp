#include <QtTest/QtTest>

#include "taskmanager.h"

#define TASK_CMDLINE_WAIT_MS 100

class TestTaskManager : public QObject
{
    Q_OBJECT

public:
    TestTaskManager();
    ~TestTaskManager();

private slots:
    void get_version();

private:
};

TestTaskManager::TestTaskManager()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // This is to "warm up" the command-line tasks.
    TaskManager *task = new TaskManager();
    task->getTarsnapVersion("");
    QTest::qWait(2 * TASK_CMDLINE_WAIT_MS);
    delete task;
}

TestTaskManager::~TestTaskManager()
{
}

void TestTaskManager::get_version()
{
    TaskManager *task = new TaskManager();
    QSignalSpy   sig_ver(task, SIGNAL(tarsnapVersion(QString)));
    QString      ver_str;

    // We start off with no version signal.
    QVERIFY(sig_ver.count() == 0);

    // Get version number (assume it's in $PATH)
    task->getTarsnapVersion("");
    QTest::qWait(TASK_CMDLINE_WAIT_MS);
    QVERIFY(sig_ver.count() == 1);
    ver_str = sig_ver.takeFirst().at(0).toString();

    // Verify that it's a real version number
    foreach(const QString part, ver_str.split("."))
    {
        for(int i = 0; i < part.size(); i++)
        {
            QVERIFY(part[i].isDigit());
        }
    }

    // Get a failure
    task->getTarsnapVersion("fake-dir");
    QTest::qWait(TASK_CMDLINE_WAIT_MS);
    QVERIFY(sig_ver.count() == 0);
    delete task;
}

QTEST_MAIN(TestTaskManager)
#include "main.moc"

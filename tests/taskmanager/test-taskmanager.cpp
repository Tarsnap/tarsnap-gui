#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "taskmanager.h"
#include "utils.h"

#include <TSettings.h>

#define TASK_CMDLINE_WAIT_MS 100

class TestTaskManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void get_version();
    void fail_registerMachine_command_not_found();
    void fail_registerMachine_empty_key();
};

void TestTaskManager::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // This is to "warm up" the command-line tasks.
    TaskManager *task = new TaskManager();
    task->getTarsnapVersion("");
    QTest::qWait(2 * TASK_CMDLINE_WAIT_MS);
    delete task;
}

void TestTaskManager::cleanupTestCase()
{
    TSettings::destroy();
}

void TestTaskManager::get_version()
{
    TARSNAP_CLI_OR_SKIP;

    TaskManager *task = new TaskManager();
    QSignalSpy   sig_ver(task, SIGNAL(tarsnapVersion(QString)));
    QString      ver_str;

    // We start off with no version signal.
    QVERIFY(sig_ver.count() == 0);

    // Get version number
    task->getTarsnapVersion("");
    QTest::qWait(TASK_CMDLINE_WAIT_MS);
    QVERIFY(sig_ver.count() == 1);
    ver_str = sig_ver.takeFirst().at(0).toString();

    // Verify that it's a real version number
    for(const QString &part : ver_str.split("."))
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

void TestTaskManager::fail_registerMachine_command_not_found()
{
    TaskManager *task = new TaskManager();
    QSignalSpy sig_reg(task, SIGNAL(registerMachineStatus(TaskStatus, QString)));
    QVariantList response;
    TaskStatus   status;
    QString      reason;

    // Fail to register with a non-existent tarsnap dir.
    task->registerMachine("fake-user", "fake-password", "fake-machine",
                          "fake.key", "/fake/dir",
                          "/tmp/gui-test-tarsnap-cache");
    QTest::qWait(TASK_CMDLINE_WAIT_MS);

    // Get failure message.
    QVERIFY(sig_reg.count() == 1);
    response = sig_reg.takeFirst();
    status   = response.at(0).value<TaskStatus>();
    reason   = response.at(1).toString();
    QVERIFY(status == TaskStatus::Failed);
    QVERIFY(reason == "Could not launch the command-line program");

    delete task;
}

void TestTaskManager::fail_registerMachine_empty_key()
{
    TARSNAP_CLI_OR_SKIP;

    TaskManager *task = new TaskManager();
    QSignalSpy sig_reg(task, SIGNAL(registerMachineStatus(TaskStatus, QString)));
    QVariantList response;
    TaskStatus   status;
    QString      reason;

    // Fail to register with a key that doesn't support --fsck-prune.
    task->registerMachine("fake-user", "fake-password", "fake-machine",
                          "empty.key", tarsnapPath,
                          "/tmp/gui-test-tarsnap-cache");
    QTest::qWait(TASK_CMDLINE_WAIT_MS);

    // Get failure message.
    QVERIFY(sig_reg.count() == 1);
    response = sig_reg.takeFirst();
    status   = response.at(0).value<TaskStatus>();
    reason   = response.at(1).toString();
    QVERIFY(status == TaskStatus::Failed);
    QVERIFY(reason.contains("tarsnap: Key file has unreasonable size"));

    delete task;
}

QTEST_MAIN(TestTaskManager)
#include "test-taskmanager.moc"

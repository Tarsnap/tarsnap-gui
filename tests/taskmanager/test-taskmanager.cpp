#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "tarsnaptask.h"
#include "taskmanager.h"
#include "utils.h"

#include <ConsoleLog.h>
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
    void sleep_cancel();
};

void TestTaskManager::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    ConsoleLog::initializeConsoleLog();

    // This is to "warm up" the command-line tasks.
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_ver(manager, SIGNAL(tarsnapVersionFound(QString)));
    manager->tarsnapVersionFind();

    // Wait for task to finish
    while(sig_ver.count() == 0)
        QTest::qWait(TASK_CMDLINE_WAIT_MS);
    delete manager;
}

void TestTaskManager::cleanupTestCase()
{
    TSettings::destroy();
    ConsoleLog::destroy();

    // Wait up to 1 second to delete objects scheduled with ->deleteLater()
    QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

void TestTaskManager::get_version()
{
    TARSNAP_CLI_OR_SKIP;

    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_ver(manager, SIGNAL(tarsnapVersionFound(QString)));
    QString      ver_str;

    // We start off with no version signal.
    QVERIFY(sig_ver.count() == 0);

    // Get version number
    manager->tarsnapVersionFind();
    while(sig_ver.count() == 0)
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
    TSettings settings;
    settings.setValue("tarsnap/path", "fake-dir");

    manager->tarsnapVersionFind();
    while(sig_ver.count() == 0)
        QTest::qWait(TASK_CMDLINE_WAIT_MS);
    QVERIFY(sig_ver.count() == 1);
    ver_str = sig_ver.takeFirst().at(0).toString();
    QVERIFY(ver_str.isEmpty());

    delete manager;
}

void TestTaskManager::fail_registerMachine_command_not_found()
{
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_reg(manager,
                       SIGNAL(registerMachineDone(TaskStatus, QString)));
    QVariantList response;
    TaskStatus   status;
    QString      reason;

    // Fail to register with a non-existent tarsnap dir.
    TSettings settings;
    settings.setValue("tarsnap/path", "/fake/dir");
    settings.setValue("tarsnap/user", "fake-user");
    settings.setValue("tarsnap/machine", "fake-machine");
    settings.setValue("tarsnap/key", "fake.key");
    settings.setValue("tarsnap/cache", "/tmp/gui-test-tarsnap-cache");
    manager->registerMachineDo("fake-password", false);
    while(sig_reg.count() == 0)
        QTest::qWait(TASK_CMDLINE_WAIT_MS);

    // Get failure message.
    QVERIFY(sig_reg.count() == 1);
    response = sig_reg.takeFirst();
    status   = response.at(0).value<TaskStatus>();
    reason   = response.at(1).toString();
    QVERIFY(status == TaskStatus::Failed);
    QVERIFY(reason == "Could not find the command-line program");

    delete manager;
}

void TestTaskManager::fail_registerMachine_empty_key()
{
    TARSNAP_CLI_OR_SKIP;

    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_reg(manager,
                       SIGNAL(registerMachineDone(TaskStatus, QString)));
    QVariantList response;
    TaskStatus   status;
    QString      reason;

    // Fail to register with a key that doesn't support --fsck-prune.
    TSettings settings;
    settings.setValue("tarsnap/path", tarsnapPath);
    settings.setValue("tarsnap/user", "fake-user");
    settings.setValue("tarsnap/machine", "fake-machine");
    settings.setValue("tarsnap/key", "empty.key");
    settings.setValue("tarsnap/cache", "/tmp/gui-test-tarsnap-cache");
    manager->registerMachineDo("fake-password", true);
    while(sig_reg.count() == 0)
        QTest::qWait(TASK_CMDLINE_WAIT_MS);

    // Get failure message.
    QVERIFY(sig_reg.count() == 1);
    response = sig_reg.takeFirst();
    status   = response.at(0).value<TaskStatus>();
    reason   = response.at(1).toString();
    QVERIFY(status == TaskStatus::Failed);
    QVERIFY(reason.contains("tarsnap: Key file has unreasonable size"));

    delete manager;
}

void TestTaskManager::sleep_cancel()
{
    // Set up the manager.
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_message(manager, SIGNAL(message(QString, QString)));

    // Set up a task.
    TarsnapTask *task = new TarsnapTask();
    QSignalSpy   sig_started(task, SIGNAL(started(QVariant)));
    task->setCommand("/bin/sh");
    task->setArguments(QStringList(get_script("sleep-9-exit-0.sh")));
    task->setData(QString("started-9"));

    // Start running it, wait a second.
    manager->startTask(task);
    QTest::qWait(1000);
    QVERIFY(sig_started.count() == 1);
    QVERIFY(sig_started.takeFirst().at(0).value<QString>() == "started-9");
    QVERIFY(sig_message.count() == 0);

    // Cancel it
    manager->stopTasks(false, true, false);
    QVERIFY(sig_message.count() == 1);
    QVERIFY(sig_message.takeFirst().at(0).value<QString>()
            == "Stopped running tasks.");

    QTest::qWait(1000);

    // task is deleted by the task manager
    delete manager;
}

QTEST_MAIN(TestTaskManager)
#include "test-taskmanager.moc"

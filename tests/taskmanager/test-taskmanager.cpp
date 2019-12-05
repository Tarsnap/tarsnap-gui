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
    void tarsnapVersion_fake();
    void registerMachine_fake();
    void initializeCache_fake();
};

void TestTaskManager::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    ConsoleLog::initializeConsoleLog();

    // This is to "warm up" the command-line tasks.
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_ver(manager,
                       SIGNAL(tarsnapVersionFound(TaskStatus, QString)));
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
    QSignalSpy   sig_ver(manager,
                       SIGNAL(tarsnapVersionFound(TaskStatus, QString)));
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

    // Reset proper tarsnap path after the "deliberate failure" test.
    settings.setValue("tarsnap/path", tarsnapPath);

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

/* Return 1 if the file ${filename} contains at least one instance of ${str}. */
static int grep_file(const char *const filename, const char *const str)
{
    QFile fp(filename);
    fp.open(QIODevice::ReadOnly | QIODevice::Text);
    while(!fp.atEnd())
    {
        const QByteArray line = fp.readLine();
        if(line.contains(str))
            return (1);
    }
    return (0);
}

void TestTaskManager::tarsnapVersion_fake()
{
    TaskManager *manager       = new TaskManager();
    const char   logfilename[] = TEST_DIR "/tarsnapVersion_fake.log";

    // Initialize log file.
    LOG.setFilename(logfilename);
    LOG.setWriteToFile(true);

    // Get version number, writing the command(s) to a file.
    manager->fakeNextTask();
    manager->tarsnapVersionFind();
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "tarsnap --version") == 1);

    // Clean up.
    LOG.setWriteToFile(false);
    delete manager;
}

void TestTaskManager::registerMachine_fake()
{
    TaskManager *manager       = new TaskManager();
    const char   logfilename[] = TEST_DIR "/registerMachine_fake.log";

    // Initialize log file.
    LOG.setFilename(logfilename);
    LOG.setWriteToFile(true);

    // Set up some settings.
    TSettings settings;
    settings.setValue("tarsnap/user", "username");
    settings.setValue("tarsnap/machine", "machine");
    settings.setValue("tarsnap/key", "keyfile");

    // Fake registering a new key.
    manager->fakeNextTask();
    manager->registerMachineDo("password", false);
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "tarsnap-keygen") == 1);

    // Fake using an existing key, a new cachedir.
    settings.setValue("tarsnap/key", "empty.key");
    settings.setValue("tarsnap/cache", TEST_DIR "/new-cachedir");
    manager->fakeNextTask();
    manager->registerMachineDo("password", true);
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "--fsck-prune") == 1);

    // Fake using an existing key, with a cachedir.
    settings.setValue("tarsnap/cache", TEST_DIR "/cachedir");
    manager->fakeNextTask();
    manager->registerMachineDo("password", true);
    manager->waitUntilIdle();
    // The next line is somewhat useless (since it'll catch the earlier
    // --fsck-prune), but it'll be meaningful after the next refactoring.
    QVERIFY(grep_file(logfilename, "--fsck-prune") == 1);

    // Clean up.
    LOG.setWriteToFile(false);
    delete manager;
}

void TestTaskManager::initializeCache_fake()
{
    TaskManager *manager       = new TaskManager();
    const char   logfilename[] = TEST_DIR "/initializeCache_fake.log";

    // Initialize log file.
    LOG.setFilename(logfilename);
    LOG.setWriteToFile(true);

    // Set up some settings.
    TSettings settings;
    settings.setValue("tarsnap/user", "username");
    settings.setValue("tarsnap/machine", "machine");
    settings.setValue("tarsnap/key", "keyfile");
    settings.setValue("tarsnap/version", "1.0.39");

    // Fake initialize a new cachedir.
    manager->fakeNextTask();
    settings.setValue("tarsnap/cache", TEST_DIR "/new-cachedir-again");
    manager->initializeCache();
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "--initialize-cachedir") == 1);

    // Fake using an existing cachedir.
    settings.setValue("tarsnap/cache", TEST_DIR "/cachedir");
    manager->fakeNextTask();
    manager->initializeCache();
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "--fsck-prune") == 1);

    // Clean up.
    LOG.setWriteToFile(false);
    delete manager;
}

QTEST_MAIN(TestTaskManager)
#include "test-taskmanager.moc"

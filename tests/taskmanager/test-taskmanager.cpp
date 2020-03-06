#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QThreadPool>
#include <QtTest/QtTest>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "tarsnaptask.h"
#include "taskmanager.h"
#include "utils.h"

#include <ConsoleLog.h>
#include <TSettings.h>

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
    WAIT_SIG(sig_ver);
    delete manager;
}

void TestTaskManager::cleanupTestCase()
{
    TSettings::destroy();
    ConsoleLog::destroy();

    // Wait up to 5 seconds for any running threads to stop.
    QThreadPool::globalInstance()->waitForDone(5000);
    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
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
    WAIT_SIG(sig_ver);
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
    WAIT_SIG(sig_ver);
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
    settings.setValue("tarsnap/key", "fake.key");
    settings.setValue("tarsnap/cache", "/tmp/gui-test-tarsnap-cache");
    manager->registerMachineDo("fake-password", "machinename", false);
    WAIT_SIG(sig_reg);

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
    settings.setValue("tarsnap/key", "empty.key");
    settings.setValue("tarsnap/cache", "/tmp/gui-test-tarsnap-cache");
    manager->registerMachineDo("fake-password", "machinename", true);
    WAIT_SIG(sig_reg);

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
    WAIT_SIG(sig_started);
    QVERIFY(sig_started.count() == 1);
    QVERIFY(sig_started.takeFirst().at(0).value<QString>() == "started-9");
    QVERIFY(sig_message.count() == 0);

    // Cancel it
    manager->stopTasks(false, true, false);
    QVERIFY(sig_message.count() == 1);
    QVERIFY(sig_message.takeFirst().at(0).value<QString>()
            == "Stopped running tasks.");

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
    TaskManager *manager = new TaskManager();
    const char * logfilename;

    // Initialize log file.
    LOG.setWriteToFile(true);

    // Get version number, writing the command(s) to a file.
    logfilename = TEST_DIR "/tarsnapVersion_fake.log";
    LOG.setFilename(logfilename);
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
    TaskManager *manager = new TaskManager();
    const char * logfilename;

    // Initialize log file.
    LOG.setWriteToFile(true);

    // Set up some settings.
    TSettings settings;
    settings.setValue("tarsnap/user", "username");
    settings.setValue("tarsnap/key", "keyfile");

    // Fake registering a new key, existing cachedir.
    logfilename = TEST_DIR "/registerMachine_fake_1.log";
    LOG.setFilename(logfilename);
    settings.setValue("tarsnap/cache", TEST_DIR "/cachedir");
    manager->fakeNextTask();
    manager->registerMachineDo("password", "machinename", false);
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "tarsnap-keygen") == 1);
    QVERIFY(grep_file(logfilename, "--fsck-prune") == 1);

    // Fake registering a new key, new cachedir.
    logfilename = TEST_DIR "/registerMachine_fake_2.log";
    LOG.setFilename(logfilename);
    settings.setValue("tarsnap/cache", TEST_DIR "/new-cachedir");
    manager->fakeNextTask();
    manager->registerMachineDo("password", "machinename", false);
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "tarsnap-keygen") == 1);
    QVERIFY(grep_file(logfilename, "--initialize-cachedir") == 1);

    // Fake using an existing key, new cachedir.
    logfilename = TEST_DIR "/registerMachine_fake_3.log";
    LOG.setFilename(logfilename);
    settings.setValue("tarsnap/key", "empty.key");
    settings.setValue("tarsnap/cache", TEST_DIR "/new-cachedir");
    manager->fakeNextTask();
    manager->registerMachineDo("password", "machinename", true);
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "tarsnap-keygen") == 0);
    QVERIFY(grep_file(logfilename, "--fsck-prune") == 1);

    // Fake using an existing key, existing cachedir.
    logfilename = TEST_DIR "/registerMachine_fake_4.log";
    LOG.setFilename(logfilename);
    settings.setValue("tarsnap/cache", TEST_DIR "/cachedir");
    manager->fakeNextTask();
    manager->registerMachineDo("password", "machinename", true);
    manager->waitUntilIdle();
    QVERIFY(grep_file(logfilename, "tarsnap-keygen") == 0);
    QVERIFY(grep_file(logfilename, "--fsck-prune") == 1);

    // Clean up.
    LOG.setWriteToFile(false);
    delete manager;
}

QTEST_MAIN(TestTaskManager)
#include "test-taskmanager.moc"

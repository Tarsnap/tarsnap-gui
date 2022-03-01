#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QByteArray>
#include <QChar>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QList>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QThreadPool>
#include <QUrl>
#include <QVariant>
#include <QVariantList>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "messages/backuptaskdataptr.h"
#include "messages/taskstatus.h"

#include "backuptask.h"
#include "cmdlinetask.h"
#include "taskmanager.h"
#include "taskqueuer.h"

#include "ConsoleLog.h"
#include "TSettings.h"

class TestTaskManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void taskqueuer();
    void get_version();
    void fail_registerMachine_command_not_found();
    void fail_registerMachine_empty_key();
    void sleep_cancel();
    void sleep_task();
    void sleep_task_cancel();
    void sleep_task_cancel_running_parallel();
    void sleep_task_cancel_running_series();
    void exclusive();
    void tarsnapVersion_fake();
    void registerMachine_fake();
    void backup_fake();
    void backup_interrupt_fake();
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

void TestTaskManager::taskqueuer()
{
    TaskQueuer *tq = new TaskQueuer();

    delete tq;
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
    CmdlineTask *task = new CmdlineTask();
    QSignalSpy   sig_started(task, SIGNAL(started(QVariant)));
    task->setCommand("/bin/sh");
    task->setArguments(QStringList(get_script("sleep-9-exit-0.sh")));
    task->setData(QString("started-9"));

    // Start running it, wait a second.
    manager->queueTask(task);
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

void TestTaskManager::sleep_task()
{
    // Set up the manager.
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_message(manager, SIGNAL(message(QString, QString)));

    // Set up a task and wait for it to start.
    manager->sleepSeconds(1, false);
    WAIT_SIG(sig_message);
    QVERIFY(sig_message.takeFirst().at(0).toString() == "Started sleep task.");
    sig_message.clear();

    // Wait for it to finish.
    WAIT_SIG(sig_message);
    QVERIFY(sig_message.takeFirst().at(0).toString() == "Finished sleep task.");

    // task is deleted by the task manager
    delete manager;
}

void TestTaskManager::sleep_task_cancel()
{
    // Set up the manager.
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_message(manager, SIGNAL(message(QString, QString)));

    // Set up a task and wait for it to start.
    manager->sleepSeconds(1, false);
    WAIT_SIG(sig_message);
    QVERIFY(sig_message.takeFirst().at(0).toString() == "Started sleep task.");
    sig_message.clear();

    // Cancel it.
    manager->stopTasks(false, true, false);
    WAIT_UNTIL(sig_message.count() >= 1);
    QVERIFY(sig_message.takeFirst().at(0).toString()
            == "Stopped running tasks.");

    // The previous WAIT_UNTIL might have gained 1 or 2 messages, so this one
    // might not be necessary in all cases (depending on multithreading).
    WAIT_UNTIL(sig_message.count() >= 1);
    QVERIFY(sig_message.takeFirst().at(0).toString() == "Finished sleep task.");

    // task is deleted by the task manager
    delete manager;
}

void TestTaskManager::sleep_task_cancel_running_parallel()
{
    // Check that we can run parallel tasks.
    if(QThreadPool::globalInstance()->maxThreadCount() == 1)
        QSKIP("max 1 thread");

    // Set up the manager.
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_message(manager, SIGNAL(message(QString, QString)));
    int          start_messages = 0;
    int          end_messages   = 0;

    // Set up two tasks running at the same time.
    manager->sleepSeconds(5, false);
    start_messages += 1;
    end_messages += 1;
    WAIT_UNTIL(sig_message.count() >= start_messages);

    manager->sleepSeconds(5, false);
    start_messages += 1;
    end_messages += 1;
    WAIT_UNTIL(sig_message.count() >= start_messages);

    // Cancel them.
    manager->stopTasks(false, true, false);
    end_messages += 1;
    WAIT_UNTIL(sig_message.count() >= (start_messages + end_messages));

    // Tasks are deleted by the task manager.
    delete manager;
}

void TestTaskManager::sleep_task_cancel_running_series()
{
    // Set up the manager.
    TaskManager *manager = new TaskManager();
    QSignalSpy   sig_message(manager, SIGNAL(message(QString, QString)));
    QVariant     msg;

    // Set up two tasks running one after the other.
    manager->sleepSeconds(5, true);
    manager->sleepSeconds(5, true);

    WAIT_UNTIL(sig_message.count() >= 1);
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString() == "Started sleep task.");
    QVERIFY(sig_message.count() == 0);

    // Cancel one of them.
    manager->stopTasks(false, true, false);
    WAIT_UNTIL(sig_message.count() >= 3);
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString() == "Stopped running tasks.");
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString() == "Finished sleep task.");
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString() == "Started sleep task.");
    QVERIFY(sig_message.count() == 0);

    // Cancel the other one.
    manager->stopTasks(false, true, false);
    WAIT_UNTIL(sig_message.count() >= 2);
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString() == "Stopped running tasks.");
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString() == "Finished sleep task.");
    QVERIFY(sig_message.count() == 0);

    // Tasks are deleted by the task manager.
    delete manager;
}

void TestTaskManager::exclusive()
{
    // Check that we can run parallel tasks.
    if(QThreadPool::globalInstance()->maxThreadCount() == 1)
        QSKIP("max 1 thread");

    // Set up the manager.
    TaskManager    *manager = new TaskManager();
    QSignalSpy      sig_numTasks(manager, SIGNAL(numTasks(bool, int, int)));
    QSignalSpy      sig_message(manager, SIGNAL(message(QString, QString)));
    QList<QVariant> numTasks;
    QVariant        msg;

    // Reminder: each sleep task will create these messages:
    // - "Started sleep task."
    // - "Finished sleep task".
    // and the manager will create:
    // - "Stopped running tasks."

    // Set up tasks:
    // - A: 1 exclusive.  (Messages: start 1, stop 2)
    manager->sleepSeconds(5, true);
    // - B: 2 non-exclusive.  (start 2, stop 3)
    manager->sleepSeconds(5, false);
    manager->sleepSeconds(5, false);
    // - C: 2 exclusive.  (start 1, stop 2)
    manager->sleepSeconds(5, true);
    // - D: 2 non-exclusive.  (start 2, stop 3)
    manager->sleepSeconds(5, false);
    manager->sleepSeconds(5, false);
    // - E: 0 new tasks.

    // A: wait for a task to start, then check that we only have one.
    // Remember that these messages counts are:
    //     previous stop + current start
    WAIT_UNTIL(sig_message.count() >= (0 + 1));
    QVERIFY(sig_numTasks.count() == 6);
    numTasks = sig_numTasks.takeLast();
    QVERIFY(numTasks.at(1).toInt() == 1);
    QVERIFY(numTasks.at(2).toInt() == 5);
    sig_message.clear();
    sig_numTasks.clear();

    // B: stop the currently-running task(s).  Messages: 2 end, 2 start.
    manager->stopTasks(false, true, false);
    WAIT_UNTIL(sig_message.count() >= (2 + 2));
    WAIT_UNTIL(sig_numTasks.count() >= 1);
    QVERIFY(sig_numTasks.count() == 1);
    numTasks = sig_numTasks.takeLast();
    QVERIFY(numTasks.at(1).toInt() == 2);
    QVERIFY(numTasks.at(2).toInt() == 3);
    sig_message.clear();
    sig_numTasks.clear();

    // C: Stop the currently-running task(s).  Messages: 3 end, 1 start.
    manager->stopTasks(false, true, false);
    WAIT_UNTIL(sig_message.count() >= (3 + 1));
    WAIT_UNTIL(sig_numTasks.count() >= 2);
    // Since two tasks are ending, we'll get two updateNumTasks.
    QVERIFY(sig_numTasks.count() == 2);
    numTasks = sig_numTasks.takeLast();
    QVERIFY(numTasks.at(1).toInt() == 1);
    QVERIFY(numTasks.at(2).toInt() == 2);
    sig_message.clear();
    sig_numTasks.clear();

    // D: Stop the currently-running task(s).  Messages: 2 end, 2 start.
    manager->stopTasks(false, true, false);
    WAIT_UNTIL(sig_message.count() >= (2 + 2));
    WAIT_UNTIL(sig_numTasks.count() >= 1);
    QVERIFY(sig_numTasks.count() == 1);
    numTasks = sig_numTasks.takeLast();
    QVERIFY(numTasks.at(1).toInt() == 2);
    QVERIFY(numTasks.at(2).toInt() == 0);
    sig_message.clear();
    sig_numTasks.clear();

    // E: Stop the currently-running task(s).  Messages: 3 end, 0 start.
    manager->stopTasks(false, true, false);
    WAIT_UNTIL(sig_message.count() >= (3 + 0));
    WAIT_UNTIL(sig_numTasks.count() >= 2);
    QVERIFY(sig_numTasks.count() == 2);
    numTasks = sig_numTasks.takeLast();
    QVERIFY(numTasks.at(1).toInt() == 0);
    QVERIFY(numTasks.at(2).toInt() == 0);
    sig_message.clear();
    sig_numTasks.clear();

    // Tasks are deleted by the task manager.
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
    const char  *logfilename;

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
    const char  *logfilename;

    // Sanity test: we shouldn't have this (extra) file in the directory.
    QVERIFY(!QFile::exists("keyfile"));

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

void TestTaskManager::backup_fake()
{
    TaskManager *manager = new TaskManager();
    const char  *logfilename;
    QSignalSpy   sig_numTasks(manager, SIGNAL(numTasks(bool, int, int)));

    QList<QVariant> numTasks;

    // Sanity test: we shouldn't have this (extra) file in the directory.
    QVERIFY(!QFile::exists("keyfile"));

    // Initialize log file.
    LOG.setWriteToFile(true);

    // Set up some settings.
    TSettings settings;
    settings.setValue("tarsnap/user", "username");
    settings.setValue("tarsnap/key", "keyfile");
    settings.setValue("tarsnap/cache", TEST_DIR "/cachedir");

    // Create backup task.
    BackupTaskDataPtr btd(new BackupTaskData);
    btd->setName("this_backup");
    QList<QUrl> testdir_urls({QUrl("file://" TEST_DIR "/file1")});
    btd->setUrls(testdir_urls);

    // Fake creating an archive.
    logfilename = TEST_DIR "/backup_fake_1.log";
    LOG.setFilename(logfilename);
    manager->fakeNextTask();
    manager->backupNow(btd);

    // Check that numTasks is correct.
    QVERIFY(sig_numTasks.count() == 1);
    numTasks = sig_numTasks.takeFirst();
    QVERIFY(numTasks.at(0).toBool() == true);

    // Wait.
    manager->waitUntilIdle();

    // Check that numTasks is correct (again).
    QVERIFY(sig_numTasks.count() == 1);
    numTasks = sig_numTasks.takeFirst();
    QVERIFY(numTasks.at(0).toBool() == false);

    // Check logfile.
    QVERIFY(grep_file(logfilename, "tarsnap") == 1);
    QVERIFY(grep_file(logfilename, "this_backup") == 1);
    QVERIFY(grep_file(logfilename, "file1") == 1);

    // Clean up.
    LOG.setWriteToFile(false);
    delete manager;
}

void TestTaskManager::backup_interrupt_fake()
{
    TARSNAP_CLI_OR_SKIP;

    TaskManager *manager = new TaskManager();
    const char  *logfilename;
    QSignalSpy   sig_message(manager, SIGNAL(message(QString, QString)));
    QSignalSpy   sig_numTasks(manager, SIGNAL(numTasks(bool, int, int)));
    QVariant     msg;

    QList<QVariant> numTasks;

    // Sanity test: we shouldn't have this (extra) file in the directory.
    QVERIFY(!QFile::exists("keyfile"));

    // Initialize log file.
    LOG.setWriteToFile(true);

    // Set up --dry-run only.  Make sure that we don't have other
    // settings, otherwise tarsnap might complain (e.g., invalid
    // keyfile, cachedir without keyfile).
    TSettings settings;
    settings.setValue("tarsnap/key", "");
    settings.setValue("tarsnap/cache", "");
    settings.setValue("tarsnap/dry_run", "true");

    // Create backup task.  Read from $HOME so that we have enough
    // data that it'll still be going when we interrupt it.
    BackupTaskDataPtr btd(new BackupTaskData);
    btd->setName("this_backup");
    QList<QUrl> testdir_urls({QUrl("file://" + QDir::homePath())});
    btd->setUrls(testdir_urls);

    // Set up the logfile and start the task.
    logfilename = TEST_DIR "/backup_fake_interrupt_1.log";
    LOG.setFilename(logfilename);
    manager->backupNow(btd);

    // Check that numTasks is correct.
    QVERIFY(sig_numTasks.count() == 1);
    numTasks = sig_numTasks.takeFirst();
    QVERIFY(numTasks.at(0).toBool() == true);

    // Wait for it to start.
    WAIT_UNTIL(sig_message.count() >= 2);
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString().contains("queued"));
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString().contains("is running"));

    // Wait another little bit.  This should be enough for the
    // tarsnap client to start reading data, but not long enough
    // for it to finish.
    QTest::qWait(500);

    // Bail if we finish early.
    if(sig_numTasks.count() > 0)
        QSKIP("Finished backup too soon");

    // Interrupt the backup.
    manager->stopTasks(true, false, false);

    // Check that we interrupted it.
    WAIT_UNTIL(sig_message.count() >= 1);
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString() == "Interrupting current backup.");

    // Check that we reported that the backup is completed.
    WAIT_UNTIL(sig_message.count() >= 1);
    msg = sig_message.takeFirst().at(0);
    QVERIFY(msg.toString().contains("completed"));

    // Check that numTasks is correct (after stopping).
    WAIT_UNTIL(sig_numTasks.count() >= 1);
    numTasks = sig_numTasks.takeFirst();
    QVERIFY(numTasks.at(0).toBool() == false);

    // Check the logfile.
    QVERIFY(grep_file(logfilename, "quit signal received") == 1);

    // Clean up.
    LOG.setWriteToFile(false);
    delete manager;
}

QTEST_MAIN(TestTaskManager)
WARNINGS_DISABLE
#include "test-taskmanager.moc"
WARNINGS_ENABLE

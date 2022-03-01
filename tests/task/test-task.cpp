#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDebug>
#include <QList>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QVariant>
WARNINGS_ENABLE

#include "ConsoleLog.h"

#include "../qtest-platform.h"

#include "cmdlinetask.h"

class TestTask : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void sleep_ok();
    void sleep_monitor();
    void sleep_fail();
    void sleep_fail_stderr();
    void sleep_crash();
    void sleep_filenotfound();
    void cmd_filenotfound();
};

void TestTask::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    ConsoleLog::initializeConsoleLog();
}

void TestTask::cleanupTestCase()
{
    ConsoleLog::destroy();

    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

#define RUN_SCRIPT(scriptname, monitor)                                        \
    CmdlineTask *task = new CmdlineTask();                                     \
    if(monitor)                                                                \
    {                                                                          \
        task->setMonitorOutput();                                              \
        connect(task, &CmdlineTask::outputStdout,                              \
                [](const QString &msg) { qDebug() << msg; });                  \
    }                                                                          \
                                                                               \
    QSignalSpy sig_started(task, SIGNAL(started(QVariant)));                   \
    QSignalSpy sig_fin(task,                                                   \
                       SIGNAL(finished(QVariant, int, QString, QString)));     \
    QSignalSpy sig_dequeue(task, SIGNAL(dequeue()));                           \
                                                                               \
    task->setCommand("/bin/sh");                                               \
    task->setArguments(QStringList(get_script(scriptname)));                   \
    task->run();                                                               \
    QVERIFY(sig_started.count() == 1);                                         \
    QVERIFY(sig_dequeue.count() == 1);                                         \
    delete task

void TestTask::sleep_ok()
{
    RUN_SCRIPT("sleep-1-exit-0.sh", false);
    // We got a "finished" signal, with exit code 0.
    QVERIFY(sig_fin.count() == 1);
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == 0);
}

void TestTask::sleep_monitor()
{
    RUN_SCRIPT("sleep-1-3x-exit-0.sh", true);
    // We got a "finished" signal, with exit code 0.
    QVERIFY(sig_fin.count() == 1);
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == 0);
}

void TestTask::sleep_fail()
{
    RUN_SCRIPT("sleep-1-exit-1.sh", false);
    // We got a "finished" signal, with exit code 1.
    QVERIFY(sig_fin.count() == 1);
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == 1);
}

void TestTask::sleep_fail_stderr()
{
    RUN_SCRIPT("sleep-1-exit-1-stderr.sh", false);
    // We got a "finished" signal, with exit code 1, and text on stderr.
    QVERIFY(sig_fin.count() == 1);
    QList<QVariant> result = sig_fin.takeFirst();
    QVERIFY(result.at(1).toInt() == 1);
    QVERIFY(result.at(3).toString() == "text on stderr");
}

void TestTask::sleep_crash()
{
    RUN_SCRIPT("sleep-1-crash.sh", false);
    // Despite the crash, we should still get a "finished" signal.
    QVERIFY(sig_fin.count() == 1);
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == EXIT_CRASHED);
}

void TestTask::sleep_filenotfound()
{
    // This script should not exist.
    RUN_SCRIPT("sleep-1-filenotfound.sh", false);
    // We got a "finished" signal, with sh exit code 127 ("command not found").
    QVERIFY(sig_fin.count() == 1);
#if defined(Q_OS_FREEBSD)
    // Bug in FreeBSD
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == 2);
#else
    // POSIX standard error code for "command not found"
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == 127);
#endif
}

void TestTask::cmd_filenotfound()
{
    CmdlineTask *task = new CmdlineTask();
    QSignalSpy   sig_started(task, SIGNAL(started(QVariant)));
    QSignalSpy sig_fin(task, SIGNAL(finished(QVariant, int, QString, QString)));
    QSignalSpy sig_dequeue(task, SIGNAL(dequeue()));

    task->setCommand("/fake/dir/fake-cmd");
    task->run();
    // We got a "finished" signal, with exit code EXIT_DID_NOT_START.
    QVERIFY(sig_fin.count() == 1);
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == EXIT_CMD_NOT_FOUND);

    delete task;
}

QTEST_MAIN(TestTask)
WARNINGS_DISABLE
#include "test-task.moc"
WARNINGS_ENABLE

#include <QtTest/QtTest>

#include "tarsnaptask.h"

class TestTask : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void sleep_ok();
    void sleep_fail();
    void sleep_crash();

private:
    QString get_script(QString scriptname);
};

void TestTask::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);
}

QString TestTask::get_script(QString scriptname)
{
    QDir dir = QDir(QCoreApplication::applicationDirPath());

#ifdef Q_OS_OSX
    // Handle the OSX app bundle.
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif

    return dir.absolutePath() + "/" + scriptname;
}

#define RUN_SCRIPT(scriptname)                                                 \
    TarsnapTask *task = new TarsnapTask();                                     \
    QSignalSpy   sig_started(task, SIGNAL(started(QVariant)));                 \
    QSignalSpy   sig_fin(task,                                                 \
                       SIGNAL(finished(QVariant, int, QString, QString)));     \
    QSignalSpy sig_dequeue(task, SIGNAL(dequeue()));                           \
                                                                               \
    task->setCommand("/bin/sh");                                               \
    task->setArguments(QStringList(get_script(scriptname)));                   \
    task->run();                                                               \
    QVERIFY(sig_started.count() == 1);                                         \
    QVERIFY(sig_dequeue.count() == 1);                                         \
    delete task;

void TestTask::sleep_ok()
{
    RUN_SCRIPT("sleep-1-exit-0.sh");
    // We got a "finished" signal, with exit code 0.
    QVERIFY(sig_fin.count() == 1);
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == 0);
}

void TestTask::sleep_fail()
{
    RUN_SCRIPT("sleep-1-exit-1.sh");
    // We got a "finished" signal, with exit code 1.
    QVERIFY(sig_fin.count() == 1);
    QVERIFY(sig_fin.takeFirst().at(1).toInt() == 1);
}

void TestTask::sleep_crash()
{
    RUN_SCRIPT("sleep-1-crash.sh");
    // Due to the crash, we did *not* get a "finished" signal.
    QVERIFY(sig_fin.count() == 0);
}

QTEST_MAIN(TestTask)
#include "main.moc"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QSignalSpy>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QTest>
#include <QVariant>
#include <QVector>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "LogEntry.h"

#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "persistentmodel/journal.h"
#include "persistentmodel/persistentstore.h"

#include "TSettings.h"

#define SAMPLE_DATE "2012-03-04"
#define SAMPLE_DATE_2106 "2106-02-08"
#define SAMPLE_DATE_FORMAT "yyyy-MM-dd"
#define SAMPLE_MESSAGE "sample message"

#define JOURNAL_START "==Session start=="
#define JOURNAL_END "==Session end=="

class TestPersistent : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void store_basic();
    void store_write();
    void store_read();
    void store_purge();

    void journal_write();
    void journal_read();
    void journal_purge();
    void journal_year_2106();

    void archive_write();
    void archive_read();

    void job_write();
    void job_read();
};

void TestPersistent::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    PersistentStore::initializePersistentStore();
}

void TestPersistent::cleanupTestCase()
{
    TSettings::destroy();
    PersistentStore::destroy();

    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

void TestPersistent::store_basic()
{
    int ok = global_store->init();
    QVERIFY(ok);
}

void TestPersistent::store_write()
{
    QSqlQuery query = global_store->createQuery();

    if(!query.prepare("insert into journal(timestamp, log) values(?, ?)"))
        QFAIL("Failed to prepare query");

    QDateTime ts = QDateTime::fromString(SAMPLE_DATE, SAMPLE_DATE_FORMAT);
    query.addBindValue(ts.toMSecsSinceEpoch());
    query.addBindValue(SAMPLE_MESSAGE);
    if(!global_store->runQuery(query))
        QFAIL("Failed to insert value into journal");
}

void TestPersistent::store_read()
{
    QSqlQuery query = global_store->createQuery();

    if(!query.prepare("select * from journal"))
        QFAIL("Failed to prepare query");

    if(!global_store->runQuery(query))
    {
        QFAIL("Failed to get values from journal");
    }
    else
    {
        while(query.next())
        {
            QVariant  ts_val = query.value(query.record().indexOf("timestamp"));
            QDateTime timestamp =
                QDateTime::fromMSecsSinceEpoch(ts_val.value<qint64>());
            QString log = query.value(query.record().indexOf("log")).toString();
            QVERIFY(timestamp.toString(SAMPLE_DATE_FORMAT) == SAMPLE_DATE);
            QVERIFY(log == SAMPLE_MESSAGE);
        }
    }
}

void TestPersistent::store_purge()
{
    QVERIFY(global_store->initialized() == true);

    global_store->purge();
    QVERIFY(global_store->initialized() == false);
}

void TestPersistent::journal_write()
{
    // Initialize the store
    int ok = global_store->init();
    QVERIFY(ok);

    // Prep
    Journal *journal = new Journal();
    journal->load();

    // Write a message
    journal->logMessage(SAMPLE_MESSAGE);

    // Clean up
    delete journal;
}

void TestPersistent::journal_read()
{
    // Prep
    Journal   *journal = new Journal();
    QSignalSpy sig_journal(journal, SIGNAL(journal(QVector<LogEntry>)));
    journal->load();

    // Get the journal
    journal->getJournal();
    QVariant          log_var = sig_journal.takeFirst().takeFirst();
    QVector<LogEntry> log     = log_var.value<QVector<LogEntry>>();

    // Check the values
    QVERIFY(log.at(0).message == JOURNAL_START);
    QVERIFY(log.at(1).message == SAMPLE_MESSAGE);
    QVERIFY(log.at(2).message == JOURNAL_END);
    QVERIFY(log.at(3).message == JOURNAL_START);

    // Clean up
    delete journal;
}

void TestPersistent::journal_purge()
{
    // Prep
    Journal   *journal = new Journal();
    QSignalSpy sig_journal(journal, SIGNAL(journal(QVector<LogEntry>)));
    journal->load();

    // Purge (delete) journal
    journal->purge();

    // Get the journal
    journal->getJournal();
    QVariant          log_var = sig_journal.takeFirst().takeFirst();
    QVector<LogEntry> log     = log_var.value<QVector<LogEntry>>();

    // Check the values
    QVERIFY(log.count() == 0);

    // Clean up
    delete journal;
}

void TestPersistent::journal_year_2106()
{
    // Write date & message
    {
        // Prep
        QSqlQuery query = global_store->createQuery();

        // Prepare date and message
        if(!query.prepare("insert into journal(timestamp, log) values(?, ?)"))
            QFAIL("Failed to prepare query");
        QDateTime ts =
            QDateTime::fromString(SAMPLE_DATE_2106, SAMPLE_DATE_FORMAT);
        query.addBindValue(ts.toMSecsSinceEpoch() / 1000);
        query.addBindValue(SAMPLE_MESSAGE);

        // Write date and message
        if(!global_store->runQuery(query))
            QFAIL("Failed to insert value into journal");
    }

    // Read date
    {
        // Prep
        Journal   *journal = new Journal();
        QSignalSpy sig_journal(journal, SIGNAL(journal(QVector<LogEntry>)));
        journal->load();

        // Get the journal
        journal->getJournal();
        QVariant          log_var = sig_journal.takeFirst().takeFirst();
        QVector<LogEntry> log     = log_var.value<QVector<LogEntry>>();

        // Check the values
        QDateTime timestamp = log.at(1).timestamp;
        QVERIFY(timestamp.toString(SAMPLE_DATE_FORMAT) == SAMPLE_DATE_2106);

        // Clean up
        delete journal;
    }
}

void TestPersistent::archive_write()
{
    // Initialize the store
    int ok = global_store->initialized();
    QVERIFY(ok);

    // Prep
    Archive *archive = new Archive();
    archive->setName("archive1");

    // Write a message
    archive->save();

    // Clean up
    delete archive;
}

void TestPersistent::archive_read()
{
    // Initialize the store
    int ok = global_store->initialized();
    QVERIFY(ok);

    // Make a query
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare("select * from archives"))
        QFAIL("Failed to prepare query");

    // Get values
    if(!global_store->runQuery(query))
        QFAIL("Failed to get values from journal");
    query.next();

    // Check the first (only) archive name.
    QString name = query.value(0).toString();
    QVERIFY(name == "archive1");

    // That should be all
    QVERIFY(query.next() == false);
}

void TestPersistent::job_write()
{
    // Initialize the store
    int ok = global_store->initialized();
    QVERIFY(ok);

    // Prep
    Job *job = new Job();
    job->setName("job1");

    // Write
    job->save();

    // Clean up
    delete job;
}

void TestPersistent::job_read()
{
    // Initialize the store
    int ok = global_store->initialized();
    QVERIFY(ok);

    // Make a query
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare("select * from jobs"))
        QFAIL("Failed to prepare query");

    // Get values
    if(!global_store->runQuery(query))
        QFAIL("Failed to get values from jobs storage");
    query.next();

    // Check the first (only) archive name.
    QString name = query.value(0).toString();
    QVERIFY(name == "job1");

    // That should be all
    QVERIFY(query.next() == false);
}

QTEST_MAIN(TestPersistent)
WARNINGS_DISABLE
#include "test-persistent.moc"
WARNINGS_ENABLE

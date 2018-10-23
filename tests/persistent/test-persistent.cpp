#include <QtTest/QtTest>

#include "persistentmodel/persistentstore.h"

#include <TSettings.h>

#define SAMPLE_DATE "2012-03-04"
#define SAMPLE_DATE_FORMAT "yyyy-MM-dd"
#define SAMPLE_MESSAGE "sample message"

class TestPersistent : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void store();
    void store_write();
    void store_read();
    void store_purge();
};

void TestPersistent::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);
}

void TestPersistent::cleanupTestCase()
{
    TSettings::destroy();
}

void TestPersistent::store()
{
    PersistentStore &store = PersistentStore::instance();
    (void)store;
}

void TestPersistent::store_write()
{
    PersistentStore &store = PersistentStore::instance();
    QSqlQuery        query = store.createQuery();

    if(!query.prepare("insert into journal(timestamp, log) values(?, ?)"))
        QFAIL("Failed to prepare query");

    QDateTime ts = QDateTime::fromString(SAMPLE_DATE, SAMPLE_DATE_FORMAT);
    query.addBindValue(ts.toMSecsSinceEpoch());
    query.addBindValue(SAMPLE_MESSAGE);
    if(!store.runQuery(query))
        QFAIL("Failed to insert value into journal");
}

void TestPersistent::store_read()
{
    PersistentStore &store = PersistentStore::instance();
    QSqlQuery        query = store.createQuery();

    if(!query.prepare("select * from journal"))
        QFAIL("Failed to prepare query");

    if(!store.runQuery(query))
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
    PersistentStore &store = PersistentStore::instance();
    QVERIFY(store.initialized() == true);

    store.purge();
    QVERIFY(store.initialized() == false);
}

QTEST_MAIN(TestPersistent)
#include "test-persistent.moc"

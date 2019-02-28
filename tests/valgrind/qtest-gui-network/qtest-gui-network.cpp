#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QNetworkAccessManager>
WARNINGS_ENABLE

class TestQTestNetwork : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();

    void pl_networkaccessmanager();
    void pl_networkaccessmanager_status();
    void pl_networkaccessmanager_repeated();
};

void TestQTestNetwork::pl_nothing()
{
}

void TestQTestNetwork::pl_networkaccessmanager()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QTest::qWait(200);
    delete nam;
}

void TestQTestNetwork::pl_networkaccessmanager_status()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager();

    QTest::qWait(200);
    nam->networkAccessible();
    QTest::qWait(200);

    delete nam;
}

void TestQTestNetwork::pl_networkaccessmanager_repeated()
{
    // QNetworkAccessManger has an unpredictable interactions with dbus,
    // causing variable memory leaks.  This is my attempt at catching them.
    for(int i = 0; i < 10; i++)
    {
        pl_networkaccessmanager_status();
        QTest::qWait(200);
    }
}

QTEST_MAIN(TestQTestNetwork)
#include "qtest-gui-network.moc"

#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>
#include <QThread>
#include <QtTest/QtTest>
WARNINGS_ENABLE

class TestQTestNetwork : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();

    void pl_networkaccessmanager();
    void pl_networkaccessmanager_wait();
    void pl_network_config();
    void pl_networkaccessmanager_status();
    void pl_networkaccessmanager_repeated();

private:
    void pl_networkaccessmanager_status_not_solo();
};

void TestQTestNetwork::pl_nothing()
{
}

void TestQTestNetwork::pl_networkaccessmanager()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    delete nam;
}

void TestQTestNetwork::pl_networkaccessmanager_wait()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager();
    QThread::msleep(200);
    delete nam;
}

void TestQTestNetwork::pl_network_config()
{
    QNetworkConfigurationManager *ncm = new QNetworkConfigurationManager;
    delete ncm;
}

void TestQTestNetwork::pl_networkaccessmanager_status_not_solo()
{
    QNetworkAccessManager *nam = new QNetworkAccessManager();

    QThread::msleep(200);
    nam->networkAccessible();
    QThread::msleep(200);

    delete nam;
}

void TestQTestNetwork::pl_networkaccessmanager_status()
{
    pl_networkaccessmanager_status_not_solo();
}

void TestQTestNetwork::pl_networkaccessmanager_repeated()
{
    // QNetworkAccessManger has an unpredictable interactions with dbus,
    // causing variable memory leaks.  This is my attempt at catching them.
    for(int i = 0; i < 10; i++)
    {
        pl_networkaccessmanager_status_not_solo();
        QThread::msleep(200);
    }
}

QTEST_MAIN(TestQTestNetwork)
#include "qtest-gui-network.moc"

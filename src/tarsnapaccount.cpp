#include "tarsnapaccount.h"
#include "debug.h"
#include "ui_logindialog.h"

#include <QNetworkRequest>
#include <QNetworkReply>

#define URL_ACTIVITY "https://www.tarsnap.com/manage.cgi?address=%1&password=%2&action=activity&format=csv"
#define URL_MACHINE_ACTIVITY "https://www.tarsnap.com/manage.cgi?address=%1&password=%2&action=subactivity&mid=0"

TarsnapAccount::TarsnapAccount(QObject *parent) : QObject(parent), _user(),
    _nam(this)
{

}

QString TarsnapAccount::user() const
{
    return _user;
}

void TarsnapAccount::setUser(const QString &user)
{
    _user = user;
}

void TarsnapAccount::getAccountInfo()
{
    QDialog login;
    Ui::loginDialog loginUi;
    loginUi.setupUi(&login);
    loginUi.textLabel->setText(loginUi.textLabel->text().arg(_user));
    if(login.exec())
    {
        QNetworkRequest activityReq;
//        QSslConfiguration conf = QSslConfiguration::defaultConfiguration();
//        conf.setProtocol(QSsl::TlsV1_2);
//        activityReq.setSslConfiguration(conf);
        QString url(URL_ACTIVITY);
        url = url.arg(QString(QUrl::toPercentEncoding(_user)));
        url = url.arg(QString(QUrl::toPercentEncoding(loginUi.passwordLineEdit->text())));
        activityReq.setUrl(url);
        activityReq.setRawHeader("User-Agent", (qApp->applicationName() + " " + qApp->applicationVersion()).toLatin1());
        QNetworkReply *activityReply = _nam.get(activityReq);
        connect(activityReply, SIGNAL(finished()), this, SLOT(readActivityCSV()));
        connect(activityReply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(networkError(QNetworkReply::NetworkError)));
        connect(activityReply, SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(sslError(QList<QSslError>)));
        DEBUG << "HERE.";
    }
}

void TarsnapAccount::displayMachineActivity()
{

}

void TarsnapAccount::readActivityCSV()
{
    QNetworkReply *activityReply = qobject_cast<QNetworkReply*>(sender());
    DEBUG << activityReply->readAll();
    activityReply->close();
    activityReply->deleteLater();
}

void TarsnapAccount::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    DEBUG << reply->errorString();
}

void TarsnapAccount::sslError(QList<QSslError> errors)
{
    foreach (QSslError error, errors)
    {
        DEBUG << error.errorString();
    }
}

QString TarsnapAccount::getMachine() const
{
    return _machine;
}

void TarsnapAccount::setMachine(const QString &machine)
{
    _machine = machine;
}


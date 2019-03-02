#include "tarsnapaccount.h"
#include "debug.h"
#include "utils.h"

#include <QNetworkReply>
#include <QNetworkRequest>

#include <TSettings.h>

#define URL_ACTIVITY                                                           \
    "https://www.tarsnap.com/"                                                 \
    "manage.cgi?address=%1&password=%2&action=activity&format=csv"
#define URL_MACHINE_ACTIVITY                                                   \
    "https://www.tarsnap.com/"                                                 \
    "manage.cgi?address=%1&password=%2&action=subactivity&mid=%3&format=csv"

#define USER_AGENT "Tarsnap " APP_VERSION

TarsnapAccount::TarsnapAccount()
{
    _nam = new QNetworkAccessManager(this);
}

TarsnapAccount::~TarsnapAccount()
{
    delete _nam;
}

void TarsnapAccount::getAccountInfo(bool    displayActivity,
                                    bool    displayMachineActivity,
                                    QString password)
{
    TSettings settings;
    _user    = settings.value("tarsnap/user", "").toString();
    _machine = settings.value("tarsnap/machine", "").toString();

    QString getActivity(URL_ACTIVITY);
    getActivity = getActivity.arg(QString(QUrl::toPercentEncoding(_user)),
                                  QString(QUrl::toPercentEncoding(password)));
    QNetworkReply *activityReply = tarsnapRequest(getActivity);
    connect(activityReply, &QNetworkReply::finished, [=]() {
        QByteArray replyData = readReply(activityReply);
        emit       possibleWarning(replyData);
        parseCredit(replyData);
        if(displayActivity)
            emit gotTable(replyData, tr("Account activity"));
    });
    _machineId = settings.value("tarsnap/key_id", 0).toULongLong();
    if(_machineId)
    {
        QString machineActivity(URL_MACHINE_ACTIVITY);
        QString hexId("%1");
        hexId = hexId.arg(_machineId, 16, 16, QLatin1Char('0'));
        machineActivity =
            machineActivity.arg(QString(QUrl::toPercentEncoding(_user)),
                                QString(QUrl::toPercentEncoding(password)),
                                QString(QUrl::toPercentEncoding(hexId)));
        QNetworkReply *machineActivityReply = tarsnapRequest(machineActivity);
        connect(machineActivityReply, &QNetworkReply::finished, [=]() {
            QByteArray replyData = readReply(machineActivityReply);
            parseLastMachineActivity(replyData);
            if(displayMachineActivity)
                emit gotTable(replyData, tr("Machine activity"));
        });
    }
}

void TarsnapAccount::parseCredit(QString csv)
{
    if(csv.isEmpty() || !csv.startsWith("RECTYPE"))
        return;
    QRegExp lastBalanceRx("^(Balance.+)$", Qt::CaseInsensitive,
                          QRegExp::RegExp2);
    QString lastBalance;
    for(const QString &line :
        csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts))
    {
        if(0 == lastBalanceRx.indexIn(line))
            lastBalance = line;
    }

    if(!lastBalance.isEmpty())
    {
        QStringList fields = lastBalance.split(',', QString::SkipEmptyParts);
        if(fields.count() != 3)
        {
            DEBUG << "Invalid CSV.";
            return;
        }
        emit accountCredit(fields.last().toDouble(),
                           QDate::fromString(fields[1], Qt::ISODate));
    }
}

void TarsnapAccount::parseLastMachineActivity(QString csv)
{
    if(csv.isEmpty() || !csv.startsWith("DATE"))
        return;
    QString lastLine =
        csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts).last();
    emit lastMachineActivity(lastLine.split(',', QString::SkipEmptyParts));
}

QNetworkReply *TarsnapAccount::tarsnapRequest(QString url)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", USER_AGENT);
    QNetworkReply *reply = _nam->get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(networkError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::sslErrors, this, &TarsnapAccount::sslError);
    return reply;
}

QByteArray TarsnapAccount::readReply(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    reply->close();
    reply->deleteLater();
    return data;
}

void TarsnapAccount::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    DEBUG << reply->errorString();
}

void TarsnapAccount::sslError(QList<QSslError> errors)
{
    for(const QSslError &error : errors)
    {
        DEBUG << error.errorString();
    }
}

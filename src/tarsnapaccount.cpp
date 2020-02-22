#include "tarsnapaccount.h"

WARNINGS_DISABLE
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
WARNINGS_ENABLE

#include "debug.h"
#include "utils.h"

#include <TSettings.h>

#define URL_MANAGE "https://www.tarsnap.com/manage.cgi"

#define USER_AGENT "Tarsnap " APP_VERSION

TarsnapAccount::TarsnapAccount()
{
    _nam = new QNetworkAccessManager();
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
    QUrlQuery post;
    _user = settings.value("tarsnap/user", "").toString();

    // Set up activity query
    post.addQueryItem("address", QUrl::toPercentEncoding(_user));
    post.addQueryItem("password", QUrl::toPercentEncoding(password));
    post.addQueryItem("action", "activity");
    post.addQueryItem("format", "csv");

    // Send and receive activity query
    QNetworkReply *activityReply = tarsnapRequest(post);
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
        QString hexId("%1");
        hexId = hexId.arg(_machineId, 16, 16, QLatin1Char('0'));
        // Set up machine activity query
        post.addQueryItem("address", QUrl::toPercentEncoding(_user));
        post.addQueryItem("password", QUrl::toPercentEncoding(password));
        post.addQueryItem("action", "subactivity");
        post.addQueryItem("mid", QUrl::toPercentEncoding(hexId));
        post.addQueryItem("format", "csv");
        // Send and receive machine activity query
        QNetworkReply *machineActivityReply = tarsnapRequest(post);
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

QNetworkReply *TarsnapAccount::tarsnapRequest(QUrlQuery post)
{
    QNetworkRequest request;
    request.setUrl(QUrl(URL_MANAGE));
    request.setRawHeader("User-Agent", USER_AGENT);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    QNetworkReply *reply = _nam->post(request, post.query().toUtf8());
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

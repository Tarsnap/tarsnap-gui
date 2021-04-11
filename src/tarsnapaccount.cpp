#include "tarsnapaccount.h"

WARNINGS_DISABLE
#include <QDate>
#include <QLatin1Char>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QSslError>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include "debug.h"

#include "TSettings.h"

#include "compat.h"

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

void TarsnapAccount::getAccountInfo(bool           displayActivity,
                                    bool           displayMachineActivity,
                                    const QString &password)
{
    TSettings settings;
    QUrlQuery post;

    const QString user = settings.value("tarsnap/user", "").toString();

    // Set up activity query
    post.addQueryItem("address", QUrl::toPercentEncoding(user));
    post.addQueryItem("password", QUrl::toPercentEncoding(password));
    post.addQueryItem("action", "activity");
    post.addQueryItem("format", "csv");

    // Send and receive activity query
    QNetworkReply *activityReply = tarsnapRequest(post);
    connect(activityReply, &QNetworkReply::finished,
            [this, activityReply, displayActivity]() {
                QByteArray replyData = readReply(activityReply);
                emit       possibleWarning(replyData);
                parseCredit(replyData);
                if(displayActivity)
                    emit gotTable(replyData, tr("Account activity"));
            });

    const quint64 machineId = settings.value("tarsnap/key_id", 0).toULongLong();
    if(machineId)
    {
        QString hexId("%1");
        hexId = hexId.arg(machineId, 16, 16, QLatin1Char('0'));
        // Set up machine activity query
        post.addQueryItem("address", QUrl::toPercentEncoding(user));
        post.addQueryItem("password", QUrl::toPercentEncoding(password));
        post.addQueryItem("action", "subactivity");
        post.addQueryItem("mid", QUrl::toPercentEncoding(hexId));
        post.addQueryItem("format", "csv");
        // Send and receive machine activity query
        QNetworkReply *machineActivityReply = tarsnapRequest(post);
        connect(machineActivityReply, &QNetworkReply::finished,
                [this, machineActivityReply, displayMachineActivity]() {
                    QByteArray replyData = readReply(machineActivityReply);
                    parseLastMachineActivity(replyData);
                    if(displayMachineActivity)
                        emit gotTable(replyData, tr("Machine activity"));
                });
    }
}

void TarsnapAccount::parseCredit(const QString &csv)
{
    if(csv.isEmpty() || !csv.startsWith("RECTYPE"))
        return;
    QRegExp lastBalanceRx("^(Balance.+)$", Qt::CaseInsensitive,
                          QRegExp::RegExp2);
    QString lastBalance;
    for(const QString &line : csv.split(QRegExp("[\r\n]"), SKIP_EMPTY_PARTS))
    {
        if(0 == lastBalanceRx.indexIn(line))
            lastBalance = line;
    }

    if(!lastBalance.isEmpty())
    {
        QStringList fields = lastBalance.split(',', SKIP_EMPTY_PARTS);
        if(fields.count() != 3)
        {
            DEBUG << "Invalid CSV.";
            return;
        }
        emit accountCredit(fields.last().toDouble(),
                           QDate::fromString(fields[1], Qt::ISODate));
    }
}

void TarsnapAccount::parseLastMachineActivity(const QString &csv)
{
    if(csv.isEmpty() || !csv.startsWith("DATE"))
        return;
    QString lastLine = csv.split(QRegExp("[\r\n]"), SKIP_EMPTY_PARTS).last();
    emit    lastMachineActivity(lastLine.split(',', SKIP_EMPTY_PARTS));
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

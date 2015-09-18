#include "tarsnapaccount.h"
#include "debug.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTableWidget>

#define URL_ACTIVITY "https://www.tarsnap.com/manage.cgi?address=%1&password=%2&action=activity&format=csv"
#define URL_MACHINE_ACTIVITY "https://www.tarsnap.com/manage.cgi?address=%1&password=%2&action=subactivity&mid=0&format=csv"

TarsnapAccount::TarsnapAccount(QWidget *parent) : QDialog(parent), _user(),
    _nam(this)
{
    _ui.setupUi(this);
}

QString TarsnapAccount::user() const
{
    return _user;
}

void TarsnapAccount::setUser(const QString &user)
{
    _user = user;
    _ui.textLabel->setText(_ui.textLabel->text().arg(_user));
}

void TarsnapAccount::getAccountInfo()
{
    if(exec())
    {
        QString activity(URL_ACTIVITY);
        activity = activity.arg(QString(QUrl::toPercentEncoding(_user)));
        activity = activity.arg(QString(QUrl::toPercentEncoding(_ui.passwordLineEdit->text())));
        connect(tarsnapRequest(activity), SIGNAL(finished()), this, SLOT(readActivityCSV()));
        QString machineActivity(URL_MACHINE_ACTIVITY);
        machineActivity = machineActivity.arg(QString(QUrl::toPercentEncoding(_user)));
        machineActivity = machineActivity.arg(QString(QUrl::toPercentEncoding(_ui.passwordLineEdit->text())));
        connect(tarsnapRequest(machineActivity), SIGNAL(finished()), this, SLOT(readMachineActivityCSV()));
    }
}

void TarsnapAccount::displayAccountActivity(QString csv)
{
    DEBUG << csv;
    if(csv.isEmpty())
        return;

    QStringList lines = csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    if(lines.count() <= 1)
        return;

    QStringList columnHeaders = lines.first().split(',', QString::SkipEmptyParts);
    lines.removeFirst();
    QTableWidget *table = new QTableWidget(lines.count(), columnHeaders.count());
    table->setHorizontalHeaderLabels(columnHeaders);
    table->horizontalHeader()->setStretchLastSection(true);

    qint64 row = 0;
    qint64 column = 0;

    foreach (QString line, lines)
    {
        foreach (QString entry, line.split(',', QString::KeepEmptyParts))
        {
            table->setItem(row, column, new QTableWidgetItem(entry));
            column++;
        }
        row++;
        column = 0;
    }
    table->show();
}

void TarsnapAccount::displayMachineActivity(QString csv)
{
    DEBUG << csv;
    if(csv.isEmpty())
        return;

    QStringList lines = csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    if(lines.count() <= 1)
        return;

    QStringList columnHeaders = lines.first().split(',', QString::SkipEmptyParts);
    lines.removeFirst();
    QTableWidget *table = new QTableWidget(lines.count(), columnHeaders.count());
    table->setHorizontalHeaderLabels(columnHeaders);
    table->horizontalHeader()->setStretchLastSection(true);

    qint64 row = 0;
    qint64 column = 0;

    foreach (QString line, lines)
    {
        foreach (QString entry, line.split(',', QString::KeepEmptyParts))
        {
            table->setItem(row, column, new QTableWidgetItem(entry));
            column++;
        }
        row++;
        column = 0;
    }
    table->show();
}

QNetworkReply* TarsnapAccount::tarsnapRequest(QString url)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent",
                         (qApp->applicationName() + " " + qApp->applicationVersion()).toLatin1());
    QNetworkReply *reply = _nam.get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(sslError(QList<QSslError>)));
    return reply;
}

void TarsnapAccount::parseCredit(QString csv)
{
    if(csv.isEmpty())
        return;
    QRegExp lastBalanceRx("^(Balance.+)$", Qt::CaseInsensitive, QRegExp::RegExp2);
    QString lastBalance;
    foreach (QString line, csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts))
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
        emit accountCredit(fields.last().toFloat(), QDate::fromString(fields[1], Qt::ISODate));
    }
}

void TarsnapAccount::readActivityCSV()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QByteArray replyData = reply->readAll();
    parseCredit(replyData);
    displayAccountActivity(replyData);
    reply->close();
    reply->deleteLater();
}

void TarsnapAccount::readMachineActivityCSV()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    displayMachineActivity(reply->readAll());
    reply->close();
    reply->deleteLater();
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


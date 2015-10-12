#include "tarsnapaccount.h"
#include "debug.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTableWidget>
#include <QMessageBox>

#define URL_ACTIVITY "https://www.tarsnap.com/manage.cgi?address=%1&password=%2&action=activity&format=csv"
#define URL_MACHINE_ACTIVITY "https://www.tarsnap.com/manage.cgi?address=%1&password=%2&action=subactivity&mid=%3&format=csv"
#define URL_LIST_MACHINES "https://www.tarsnap.com/manage.cgi?address=%1&password=%2&action=subactivity"

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
    _ui.textLabel->setText(QString("Type password for account %1:").arg(_user));
}

QString TarsnapAccount::getMachine() const
{
    return _machine;
}

void TarsnapAccount::setMachine(const QString &machine)
{
    _machine = machine;
}

void TarsnapAccount::getAccountInfo(bool displayActivity, bool displayMachineActivity)
{
    if(exec())
    {
        QString getActivity(URL_ACTIVITY);
        getActivity = getActivity.arg(QString(QUrl::toPercentEncoding(_user)),
                                      QString(QUrl::toPercentEncoding(_ui.passwordLineEdit->text())));
        QNetworkReply *activityReply = tarsnapRequest(getActivity);
        connect(activityReply, &QNetworkReply::finished,
        [=]() {
            QByteArray replyData = readReply(activityReply, true);
            parseCredit(replyData);
            if(displayActivity)
                displayCSVTable(replyData);
        });
        QString getMachineId(URL_LIST_MACHINES);
        getMachineId = getMachineId.arg(QString(QUrl::toPercentEncoding(_user)),
                                        QString(QUrl::toPercentEncoding(_ui.passwordLineEdit->text())));
        QNetworkReply *machineIdReply = tarsnapRequest(getMachineId);
        connect(machineIdReply, &QNetworkReply::finished,
        [=]() {
            QString machineId = parseMachineId(readReply(machineIdReply));
            QString machineActivity(URL_MACHINE_ACTIVITY);
            machineActivity = machineActivity.arg(QString(QUrl::toPercentEncoding(_user)),
                                                  QString(QUrl::toPercentEncoding(_ui.passwordLineEdit->text())),
                                                  QString(QUrl::toPercentEncoding(machineId)));
            QNetworkReply *machineActivityReply = tarsnapRequest(machineActivity);
            connect(machineActivityReply, &QNetworkReply::finished,
                    [=]() {
                    QByteArray replyData = readReply(machineActivityReply);
                    parseLastMachineActivity(replyData);
                    if(displayMachineActivity)
                        displayCSVTable(replyData);
            });
        });
    }
}

QString TarsnapAccount::parseMachineId(QString html)
{
    QString machineId;

    if(html.isEmpty() || !html.startsWith("<!DOCTYPE html>"))
        return machineId;

    QRegExp machineIdRx("mid=(\\d+)\">" + QRegExp::escape(_machine) + "</a>", Qt::CaseSensitive, QRegExp::RegExp2);

    if(machineIdRx.lastIndexIn(html))
        machineId = machineIdRx.cap(1);
    else
        DEBUG << "Machine id not found for machine " << _machine;

    return machineId;
}

void TarsnapAccount::parseCredit(QString csv)
{
    if(csv.isEmpty() || !csv.startsWith("RECTYPE"))
        return;
    QRegExp lastBalanceRx("^(Balance.+)$", Qt::CaseInsensitive, QRegExp::RegExp2);
    QString lastBalance;
    foreach(QString line, csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts))
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

void TarsnapAccount::parseLastMachineActivity(QString csv)
{
    if(csv.isEmpty() || !csv.startsWith("DATE"))
        return;
    QString lastLine = csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts).last();
    emit lastMachineActivity(lastLine.split(',', QString::SkipEmptyParts));
}

void TarsnapAccount::displayCSVTable(QString csv)
{
    DEBUG << csv;
    if(csv.isEmpty() || csv.startsWith("<!DOCTYPE html>"))
        return;

    QStringList lines = csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    if(lines.count() <= 1)
        return;

    QStringList columnHeaders = lines.first().split(',', QString::SkipEmptyParts);
    lines.removeFirst();

    QTableWidget *table = new QTableWidget(lines.count(), columnHeaders.count());
    table->setHorizontalHeaderLabels(columnHeaders);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QDialog *tableDialog = new QDialog(this);
    tableDialog->setAttribute( Qt::WA_DeleteOnClose, true );
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(table);
    layout->setMargin(0);
    tableDialog->setLayout(layout);

    qint64 row = 0;
    qint64 column = 0;

    foreach(QString line, lines)
    {
        foreach(QString entry, line.split(',', QString::KeepEmptyParts))
        {
            table->setItem(row, column, new QTableWidgetItem(entry));
            column++;
        }
        row++;
        column = 0;
    }
    tableDialog->show();
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

QByteArray TarsnapAccount::readReply(QNetworkReply *reply, bool warn)
{
    QByteArray data = reply->readAll();
    if(warn && data.contains("Password is incorrect; please try again."))
    {
        QMessageBox::warning(this, tr("Invalid password"),
                             tr("Password for account %1 is incorrect; please try again.").arg(_user));
    }
    reply->close();
    reply->deleteLater();
    return data;
}

void TarsnapAccount::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    DEBUG << reply->errorString();
}

void TarsnapAccount::sslError(QList<QSslError> errors)
{
    foreach(QSslError error, errors)
    {
        DEBUG << error.errorString();
    }
}

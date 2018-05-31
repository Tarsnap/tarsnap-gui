#include "tarsnapaccount.h"
#include "debug.h"
#include "utils.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTableWidget>

#define URL_ACTIVITY                                                           \
    "https://www.tarsnap.com/"                                                 \
    "manage.cgi?address=%1&password=%2&action=activity&format=csv"
#define URL_MACHINE_ACTIVITY                                                   \
    "https://www.tarsnap.com/"                                                 \
    "manage.cgi?address=%1&password=%2&action=subactivity&mid=%3&format=csv"

TarsnapAccount::TarsnapAccount(QWidget *parent) : QDialog(parent), _nam(this)
{
    _ui.setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint)
                   & ~Qt::WindowMaximizeButtonHint);
    connect(_ui.passwordLineEdit, &QLineEdit::textEdited, this, [&]() {
        _ui.loginButton->setEnabled(!_ui.passwordLineEdit->text().isEmpty());
    });
}

TarsnapAccount::~TarsnapAccount()
{
}

void TarsnapAccount::getAccountInfo(bool displayActivity,
                                    bool displayMachineActivity)
{
    QSettings settings;
    _user    = settings.value("tarsnap/user", "").toString();
    _machine = settings.value("tarsnap/machine", "").toString();
    if(Utils::tarsnapVersionMinimum("1.0.37"))
    {
        emit getKeyId(settings.value("tarsnap/key", "").toString());
    }
    else
    {
        QMessageBox::warning(
            this->parentWidget(), tr("Warning"),
            tr("You need Tarsnap CLI utils version 1.0.37 to "
               "be able to fetch machine activity. "
               "You have version %1.")
                .arg(settings.value("tarsnap/version", "").toString()));
    }
    if(_user.isEmpty() || _machine.isEmpty())
    {
        QMessageBox::warning(this->parentWidget(), tr("Warning"),
                             tr("Tarsnap user and machine name must be set."));
        return;
    }
    _ui.textLabel->setText(tr("Type password for account %1:").arg(_user));
    _ui.loginButton->setEnabled(false);
    if(exec() == QDialog::Rejected)
        return;
    QString getActivity(URL_ACTIVITY);
    getActivity = getActivity.arg(QString(QUrl::toPercentEncoding(_user)),
                                  QString(QUrl::toPercentEncoding(
                                      _ui.passwordLineEdit->text())));
    QNetworkReply *activityReply = tarsnapRequest(getActivity);
    connect(activityReply, &QNetworkReply::finished, [=]() {
        QByteArray replyData = readReply(activityReply, true);
        parseCredit(replyData);
        if(displayActivity)
            displayCSVTable(replyData, tr("Account activity"));
    });
    _machineId = settings.value("tarsnap/key_id", 0).toULongLong();
    if(_machineId)
    {
        QString machineActivity(URL_MACHINE_ACTIVITY);
        QString hexId("%1");
        hexId = hexId.arg(_machineId, 16, 16, QLatin1Char('0'));
        machineActivity =
            machineActivity.arg(QString(QUrl::toPercentEncoding(_user)),
                                QString(QUrl::toPercentEncoding(
                                    _ui.passwordLineEdit->text())),
                                QString(QUrl::toPercentEncoding(hexId)));
        QNetworkReply *machineActivityReply = tarsnapRequest(machineActivity);
        connect(machineActivityReply, &QNetworkReply::finished, [=]() {
            QByteArray replyData = readReply(machineActivityReply);
            parseLastMachineActivity(replyData);
            if(displayMachineActivity)
                displayCSVTable(replyData, tr("Machine activity"));
        });
    }
    _ui.passwordLineEdit->clear();
}

void TarsnapAccount::parseCredit(QString csv)
{
    if(csv.isEmpty() || !csv.startsWith("RECTYPE"))
        return;
    QRegExp lastBalanceRx("^(Balance.+)$", Qt::CaseInsensitive,
                          QRegExp::RegExp2);
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

void TarsnapAccount::displayCSVTable(QString csv, QString title)
{
    DEBUG << csv;
    if(csv.isEmpty() || csv.startsWith("<!DOCTYPE html>"))
        return;

    QStringList lines = csv.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    if(lines.count() <= 1)
        return;

    QStringList columnHeaders =
        lines.first().split(',', QString::SkipEmptyParts);
    lines.removeFirst();

    QDialog *csvDialog = new QDialog(this);
    csvDialog->setWindowTitle(title);
    csvDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    QTableWidget *table =
        new QTableWidget(lines.count(), columnHeaders.count(), csvDialog);
    table->setHorizontalHeaderLabels(columnHeaders);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(table);
    layout->setMargin(0);
    csvDialog->setLayout(layout);

    int row    = 0;
    int column = 0;

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
    csvDialog->show();
}

QNetworkReply *TarsnapAccount::tarsnapRequest(QString url)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader(
        "User-Agent",
        (qApp->applicationName() + " " + qApp->applicationVersion()).toLatin1());
    QNetworkReply *reply = _nam.get(request);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(networkError(QNetworkReply::NetworkError)));
    connect(reply, &QNetworkReply::sslErrors, this, &TarsnapAccount::sslError);
    return reply;
}

QByteArray TarsnapAccount::readReply(QNetworkReply *reply, bool warn)
{
    QByteArray data = reply->readAll();
    if(warn && data.contains("Password is incorrect; please try again."))
    {
        QMessageBox::warning(
            this, tr("Invalid password"),
            tr("Password for account %1 is incorrect; please try again.")
                .arg(_user));
    }
    else if(warn
            && data.contains("No user exists with the provided email "
                             "address; please try again."))
    {
        QMessageBox::warning(
            this, tr("Invalid username"),
            tr("Account %1 is invalid; please try again.").arg(_user));
    }
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
    foreach(QSslError error, errors)
    {
        DEBUG << error.errorString();
    }
}

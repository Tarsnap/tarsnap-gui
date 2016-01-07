#ifndef TARSNAPACCOUNT_H
#define TARSNAPACCOUNT_H

#include "ui_logindialog.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>

class TarsnapAccount : public QDialog
{
    Q_OBJECT

public:
    explicit TarsnapAccount(QWidget *parent = 0);

signals:
    void accountCredit(qreal credit, QDate date);
    void lastMachineActivity(QStringList activityFields);

public slots:
    void getAccountCredit();
    void getAccountInfo(bool displayActivity        = false,
                        bool displayMachineActivity = false);

protected slots:
    QString parseMachineId(QString html);
    void parseCredit(QString csv);
    void parseLastMachineActivity(QString csv);
    void displayCSVTable(QString csv, QString title);
    QNetworkReply *tarsnapRequest(QString url);
    QByteArray readReply(QNetworkReply *reply, bool warn = false);
    void networkError(QNetworkReply::NetworkError error);
    void sslError(QList<QSslError> errors);

private:
    Ui::loginDialog       _ui;
    QNetworkAccessManager _nam;
    QString               _user;
    QString               _machine;
};

#endif // TARSNAPACCOUNT_H

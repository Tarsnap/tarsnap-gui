#ifndef TARSNAPACCOUNT_H
#define TARSNAPACCOUNT_H

#include "ui_logindialog.h"

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class TarsnapAccount : public QDialog
{
    Q_OBJECT

public:
    explicit TarsnapAccount(QWidget *parent = 0);

    QString user() const;
    void setUser(const QString &user);

    QString getMachine() const;
    void setMachine(const QString &machine);

signals:
    void accountCredit(qreal credit, QDate date);
    void lastMachineActivity(QStringList activityFields);

public slots:
    void getAccountInfo(bool displayActivity = false,
                        bool displayMachineActivity = false);

protected slots:
    QString parseMachineId(QString html);
    void parseCredit(QString csv);
    void parseLastMachineActivity(QString csv);
    void displayCSVTable(QString csv);
    QNetworkReply* tarsnapRequest(QString url);
    QByteArray readReply(QNetworkReply *reply, bool warn = false);
    void networkError(QNetworkReply::NetworkError error);
    void sslError(QList<QSslError> errors);

private:
    Ui::loginDialog       _ui;
    QString               _user;
    QString               _machine;
    QNetworkAccessManager _nam;
};

#endif // TARSNAPACCOUNT_H

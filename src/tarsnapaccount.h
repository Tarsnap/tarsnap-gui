#ifndef TARSNAPACCOUNT_H
#define TARSNAPACCOUNT_H

#include "ui_logindialog.h"

#include <QDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class TarsnapAccount : public QDialog
{
    Q_OBJECT

public:
    explicit TarsnapAccount(QWidget *parent = nullptr);
    ~TarsnapAccount();

public slots:
    void getAccountInfo(bool displayActivity        = false,
                        bool displayMachineActivity = false);

signals:
    void accountCredit(qreal credit, QDate date);
    void lastMachineActivity(QStringList activityFields);
    void getKeyId(QString key);

protected slots:
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
    quint64               _machineId;

    QMessageBox _popup;
};

#endif // TARSNAPACCOUNT_H

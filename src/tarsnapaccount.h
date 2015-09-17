#ifndef TARSNAPACCOUNT_H
#define TARSNAPACCOUNT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class TarsnapAccount : public QObject
{
    Q_OBJECT

public:
    explicit TarsnapAccount(QObject *parent = 0);

    QString user() const;
    void setUser(const QString &user);

    QString getMachine() const;
    void setMachine(const QString &machine);

signals:
    void accountInfo(qreal credit, QString lastActivity);

public slots:
    void getAccountInfo();
    void displayMachineActivity();

protected slots:
    void readActivityCSV();
    void networkError(QNetworkReply::NetworkError error);
    void sslError(QList<QSslError> errors);

private:
    QString               _user;
    QString               _machine;
    QNetworkAccessManager _nam;
};

#endif // TARSNAPACCOUNT_H

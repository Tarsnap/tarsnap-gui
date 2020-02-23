#ifndef TARSNAPACCOUNT_H
#define TARSNAPACCOUNT_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QNetworkReply>
#include <QObject>
#include <QUrlQuery>
WARNINGS_ENABLE

/* Forward declaration. */
class QNetworkAccessManager;

/*!
 * \ingroup widgets-specialized
 * \brief The TarsnapAccount is a QObject which launches network connections to
 * the Tarsnap servers to get information about the user's account.
 */
class TarsnapAccount : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    explicit TarsnapAccount();
    ~TarsnapAccount();

public slots:
    //! Send a query to the Tarsnap service.
    //! \param displayActivity show the account's overall activity.
    //! \param displayMachineActivity show the activity for this machine.
    //! \param password the account password.
    void getAccountInfo(bool displayActivity, bool displayMachineActivity,
                        QString password);

signals:
    //! The amount of money in the Tarsnap account, and the latest date.
    void accountCredit(qreal credit, QDate date);
    //! The latest machine activity.
    void lastMachineActivity(QStringList activityFields);
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(QString key);

    //! Received some information to display.
    void gotTable(QString csv, QString title);
    //! Pass the reply to give a warning (if applicable).
    void possibleWarning(QByteArray reply);

protected slots:
    //! Extract the credits from the CSV in the reply; will emit \ref
    //! accountCredit.
    void parseCredit(QString csv);
    //! Extract the machine activity from the CSV in the reply; will emit \ref
    //! lastMachineActivity.
    void parseLastMachineActivity(QString csv);
    //! Send a network request and wait for a reply.
    QNetworkReply *tarsnapRequest(QUrlQuery post);
    //! Parse the network reply.
    QByteArray readReply(QNetworkReply *reply);
    //! Handle an error in the network reply.
    void networkError(QNetworkReply::NetworkError error);
    //! Handle an SSL error in the network reply.
    void sslError(QList<QSslError> errors);

private:
    QNetworkAccessManager *_nam;
    QString                _user;
    quint64                _machineId;
};

#endif // TARSNAPACCOUNT_H

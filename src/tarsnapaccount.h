#ifndef TARSNAPACCOUNT_H
#define TARSNAPACCOUNT_H

#include <QDialog>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui
{
class LoginDialog;
}

/*!
 * \ingroup widgets-specialized
 * \brief The TarsnapAccount is a QDialog which displays info about the account
 * and launches network connections to the Tarsnap servers to update that info.
 */
class TarsnapAccount : public QDialog
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSettingsWidget;
#endif

public:
    //! Constructor.
    explicit TarsnapAccount(QWidget *parent = nullptr);
    ~TarsnapAccount();

public slots:
    //! Update the account info from the Tarsnap servers, optionally showing
    //! the results QTableWidget(s).
    void getAccountInfo(bool displayActivity        = false,
                        bool displayMachineActivity = false);

    //! Check the reply for anything to warn about.
    void showWarningIfApplicable(QByteArray data);
    //! Format and display a comma-separated-value table in a QTableWidget.
    void displayCSVTable(QString csv, QString title);

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
    QNetworkReply *tarsnapRequest(QString url);
    //! Parse the network reply.
    QByteArray readReply(QNetworkReply *reply);
    //! Handle an error in the network reply.
    void networkError(QNetworkReply::NetworkError error);
    //! Handle an SSL error in the network reply.
    void sslError(QList<QSslError> errors);

private:
    Ui::LoginDialog *     _ui;
    QNetworkAccessManager _nam;
    QString               _user;
    QString               _machine;
    quint64               _machineId;

    QMessageBox _popup;

    void getAccountInfo_backend(bool displayActivity,
                                bool displayMachineActivity, QString password);
};

#endif // TARSNAPACCOUNT_H

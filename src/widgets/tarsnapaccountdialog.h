#ifndef TARSNAPACCOUNTDIALOG_H
#define TARSNAPACCOUNTDIALOG_H

#include <QDate>
#include <QDialog>
#include <QMessageBox>

namespace Ui
{
class LoginDialog;
}

class TarsnapAccount;

/*!
 * \ingroup widgets-specialized
 * \brief The TarsnapAccountDialog is a QDialog which displays info about the
 * account.
 */
class TarsnapAccountDialog : public QDialog
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSettingsWidget;
#endif

public:
    //! Constructor.
    explicit TarsnapAccountDialog(QWidget *parent = nullptr);
    ~TarsnapAccountDialog();

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

private:
    Ui::LoginDialog *_ui;
    TarsnapAccount * _ta;
    QMessageBox      _popup;
    QString          _user;
    QString          _machine;
    quint64          _machineId;
};

#endif // TARSNAPACCOUNT_H

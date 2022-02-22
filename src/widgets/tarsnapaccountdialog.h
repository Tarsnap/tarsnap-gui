#ifndef TARSNAPACCOUNTDIALOG_H
#define TARSNAPACCOUNTDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QByteArray>
#include <QDate>
#include <QDialog>
#include <QObject>
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class LoginDialog;
}
class QMessageBox;
class QWidget;
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
    ~TarsnapAccountDialog() override;

public slots:
    //! Update the account info from the Tarsnap servers, optionally showing
    //! the results QTableWidget(s).
    void getAccountInfo(bool displayActivity        = false,
                        bool displayMachineActivity = false);

    //! Check the reply for anything to warn about.
    void showWarningIfApplicable(const QByteArray &data);
    //! Format and display a comma-separated-value table in a QTableWidget.
    void displayCSVTable(const QString &csv, const QString &title);

signals:
    //! The amount of money in the Tarsnap account, and the latest date.
    void accountCredit(qreal credit, const QDate &date);
    //! The latest machine activity.
    void lastMachineActivity(const QStringList &activityFields);
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(const QString &key);

private slots:
    void processPasswordBox(int res);

private:
    Ui::LoginDialog *_ui;
    TarsnapAccount  *_ta;
    QMessageBox     *_popup;
    QString          _user;
    quint64          _machineId;

    bool _displayActivity;
    bool _displayMachineActivity;
};

#endif /* !TARSNAPACCOUNTDIALOG_H */

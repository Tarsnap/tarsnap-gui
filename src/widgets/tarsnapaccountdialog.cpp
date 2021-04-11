#include "tarsnapaccountdialog.h"

WARNINGS_DISABLE
#include <QAbstractItemView>
#include <QDate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExp>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <Qt>

#include "ui_logindialog.h"
WARNINGS_ENABLE

#include "TSettings.h"

#include "compat.h"
#include "debug.h"
#include "tarsnapaccount.h"

/* Forward declaration(s). */
class QWidget;

TarsnapAccountDialog::TarsnapAccountDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::LoginDialog), _popup(new QMessageBox())
{
    // Basic UI.
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint)
                   & ~Qt::WindowMaximizeButtonHint);
    _popup->setParent(this->parentWidget());
    _popup->setWindowModality(Qt::NonModal);

    // Connection for basic UI.
    connect(_ui->passwordLineEdit, &QLineEdit::textEdited, [this]() {
        const bool empty_password = _ui->passwordLineEdit->text().isEmpty();
        _ui->loginButton->setEnabled(!empty_password);
    });

    // Create the TarsnapAccount backend.
    _ta = new TarsnapAccount();

    // Act on password.
    connect(this, &TarsnapAccountDialog::finished, this,
            &TarsnapAccountDialog::processPasswordBox);

    // Act on query response.
    connect(_ta, &TarsnapAccount::gotTable, this,
            &TarsnapAccountDialog::displayCSVTable);
    connect(_ta, &TarsnapAccount::possibleWarning, this,
            &TarsnapAccountDialog::showWarningIfApplicable);

    // Pass signals onwards.
    connect(_ta, &TarsnapAccount::accountCredit, this,
            &TarsnapAccountDialog::accountCredit);
    connect(_ta, &TarsnapAccount::lastMachineActivity, this,
            &TarsnapAccountDialog::lastMachineActivity);
    connect(_ta, &TarsnapAccount::getKeyId, this,
            &TarsnapAccountDialog::getKeyId);
}

TarsnapAccountDialog::~TarsnapAccountDialog()
{
    delete _popup;
    delete _ta;
    delete _ui;
}

void TarsnapAccountDialog::getAccountInfo(bool displayActivity,
                                          bool displayMachineActivity)
{
    // What type of info do we want?
    _displayActivity        = displayActivity;
    _displayMachineActivity = displayMachineActivity;

    // Get the username, or warn if there isn't one.
    TSettings settings;
    _user = settings.value("tarsnap/user", "").toString();
    if(_user.isEmpty())
    {
        _popup->setWindowTitle(tr("Warning"));
        _popup->setIcon(QMessageBox::Warning);
        _popup->setText(tr("Tarsnap username must be set."));
        _popup->open();
        return;
    }

    // Send a message to get the key id and save it in "tarsnap/key_id".
    emit getKeyId(settings.value("tarsnap/key", "").toString());

    // ... while that's happening, ask the user for the password.
    _ui->textLabel->setText(tr("Type password for account %1:").arg(_user));
    _ui->loginButton->setEnabled(false);
    open();
}

void TarsnapAccountDialog::processPasswordBox(int res)
{
    // Bail (if applicable).
    if(res == QDialog::Rejected)
        return;

    // Use password to get account info.  This will only work if the key id
    // has already been saved to "tarsnap/key_id" in TSettings.
    _ta->getAccountInfo(_displayActivity, _displayMachineActivity,
                        _ui->passwordLineEdit->text());

    // Attempt to remove password from memory.  This is not as powerful as
    // libcperciva's insecure_memzero(), but looking through the API docs and
    // even the source code for QLineEdit hasn't suggested a better option.
    _ui->passwordLineEdit->clear();
}

void TarsnapAccountDialog::displayCSVTable(const QString &csv,
                                           const QString &title)
{
    DEBUG << csv;
    // Bail (if applicable).
    if(csv.isEmpty() || csv.startsWith("<!DOCTYPE html>"))
        return;

    // Split output into lines.
    QStringList lines = csv.split(QRegExp("[\r\n]"), SKIP_EMPTY_PARTS);
    if(lines.count() <= 1)
        return;

    // Extract column headers.
    QStringList columnHeaders = lines.first().split(',', SKIP_EMPTY_PARTS);
    lines.removeFirst();

    // Create new widget in which to display the table.
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

    // Add the data to the table inside the new widget.
    int row    = 0;
    int column = 0;
    for(const QString &line : lines)
    {
        for(const QString &entry : line.split(',', KEEP_EMPTY_PARTS))
        {
            table->setItem(row, column, new QTableWidgetItem(entry));
            column++;
        }
        row++;
        column = 0;
    }

    // Show the widget to the user.
    csvDialog->show();
}

void TarsnapAccountDialog::showWarningIfApplicable(const QByteArray &data)
{
    // Check for an incorrect password.
    if(data.contains("Password is incorrect; please try again."))
    {
        _popup->setWindowTitle(tr("Invalid password"));
        _popup->setIcon(QMessageBox::Warning);
        _popup->setText(
            tr("Password for account %1 is incorrect; please try again.")
                .arg(_user));
        _popup->open();
        return;
    }

    // Check for an invalid username.
    if(data.contains("No user exists with the provided email "
                     "address; please try again."))
    {
        _popup->setWindowTitle(tr("Invalid username"));
        _popup->setIcon(QMessageBox::Warning);
        _popup->setText(
            tr("Account %1 is invalid; please try again.").arg(_user));
        _popup->open();
        return;
    }
}

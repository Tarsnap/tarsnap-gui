#include "tarsnapaccountdialog.h"

WARNINGS_DISABLE
#include <QAbstractItemView>
#include <QDate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <Qt>

#include "ui_logindialog.h"
WARNINGS_ENABLE

#include "debug.h"
#include "tarsnapaccount.h"

#include "TSettings.h"

/* Forward declaration(s). */
class QWidget;

TarsnapAccountDialog::TarsnapAccountDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::LoginDialog)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint)
                   & ~Qt::WindowMaximizeButtonHint);
    connect(_ui->passwordLineEdit, &QLineEdit::textEdited, [this]() {
        _ui->loginButton->setEnabled(!_ui->passwordLineEdit->text().isEmpty());
    });

    _ta = new TarsnapAccount();

    // Act on password
    connect(this, &TarsnapAccountDialog::finished, this,
            &TarsnapAccountDialog::processPasswordBox);

    connect(_ta, &TarsnapAccount::gotTable, this,
            &TarsnapAccountDialog::displayCSVTable);
    connect(_ta, &TarsnapAccount::possibleWarning, this,
            &TarsnapAccountDialog::showWarningIfApplicable);

    // Pass signals onwards
    connect(_ta, &TarsnapAccount::accountCredit, this,
            &TarsnapAccountDialog::accountCredit);
    connect(_ta, &TarsnapAccount::lastMachineActivity, this,
            &TarsnapAccountDialog::lastMachineActivity);
    connect(_ta, &TarsnapAccount::getKeyId, this,
            &TarsnapAccountDialog::getKeyId);

    _popup.setParent(this->parentWidget());
    _popup.setWindowModality(Qt::NonModal);
}

TarsnapAccountDialog::~TarsnapAccountDialog()
{
    delete _ta;
    delete _ui;
}

void TarsnapAccountDialog::getAccountInfo(bool displayActivity,
                                          bool displayMachineActivity)
{
    _displayActivity        = displayActivity;
    _displayMachineActivity = displayMachineActivity;

    TSettings settings;
    _user = settings.value("tarsnap/user", "").toString();
    emit getKeyId(settings.value("tarsnap/key", "").toString());
    if(_user.isEmpty())
    {
        _popup.setWindowTitle(tr("Warning"));
        _popup.setIcon(QMessageBox::Warning);
        _popup.setText(tr("Tarsnap username must be set."));
        _popup.open();
        return;
    }
    _ui->textLabel->setText(tr("Type password for account %1:").arg(_user));
    _ui->loginButton->setEnabled(false);
    open();
}

void TarsnapAccountDialog::processPasswordBox(int res)
{
    if(res == QDialog::Rejected)
        return;

    _ta->getAccountInfo(_displayActivity, _displayMachineActivity,
                        _ui->passwordLineEdit->text());
    _ui->passwordLineEdit->clear();
}

void TarsnapAccountDialog::displayCSVTable(QString csv, QString title)
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

    for(const QString &line : lines)
    {
        for(const QString &entry : line.split(',', QString::KeepEmptyParts))
        {
            table->setItem(row, column, new QTableWidgetItem(entry));
            column++;
        }
        row++;
        column = 0;
    }
    csvDialog->show();
}

void TarsnapAccountDialog::showWarningIfApplicable(QByteArray data)
{
    if(data.contains("Password is incorrect; please try again."))
    {
        _popup.setWindowTitle(tr("Invalid password"));
        _popup.setIcon(QMessageBox::Warning);
        _popup.setText(
            tr("Password for account %1 is incorrect; please try again.")
                .arg(_user));
        _popup.open();
    }
    else if(data.contains("No user exists with the provided email "
                          "address; please try again."))
    {
        _popup.setWindowTitle(tr("Invalid username"));
        _popup.setIcon(QMessageBox::Warning);
        _popup.setText(
            tr("Account %1 is invalid; please try again.").arg(_user));
        _popup.open();
    }
}

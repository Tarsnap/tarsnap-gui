#include "tarsnapaccountdialog.h"

WARNINGS_DISABLE
#include <QHeaderView>
#include <QTableWidget>

#include "ui_logindialog.h"
WARNINGS_ENABLE

#include "debug.h"
#include "tarsnapaccount.h"
#include "utils.h"

#include <TSettings.h>

TarsnapAccountDialog::TarsnapAccountDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::LoginDialog)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint)
                   & ~Qt::WindowMaximizeButtonHint);
    connect(_ui->passwordLineEdit, &QLineEdit::textEdited, this, [&]() {
        _ui->loginButton->setEnabled(!_ui->passwordLineEdit->text().isEmpty());
    });

    _ta = new TarsnapAccount();

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
    TSettings settings;
    _user = settings.value("tarsnap/user", "").toString();
    if(Utils::tarsnapVersionMinimum("1.0.37"))
    {
        emit getKeyId(settings.value("tarsnap/key", "").toString());
    }
    else
    {
        _popup.setWindowTitle(tr("Warning"));
        _popup.setIcon(QMessageBox::Warning);
        _popup.setText(
            tr("You need Tarsnap CLI utils version 1.0.37 to "
               "be able to fetch machine activity. "
               "You have version %1.")
                .arg(settings.value("tarsnap/version", "").toString()));
        _popup.exec();
    }
    if(_user.isEmpty())
    {
        _popup.setWindowTitle(tr("Warning"));
        _popup.setIcon(QMessageBox::Warning);
        _popup.setText(tr("Tarsnap user and machine name must be set."));
        _popup.exec();
        return;
    }
    _ui->textLabel->setText(tr("Type password for account %1:").arg(_user));
    _ui->loginButton->setEnabled(false);
    if(exec() == QDialog::Rejected)
        return;

    _ta->getAccountInfo(displayActivity, displayMachineActivity,
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
        _popup.exec();
    }
    else if(data.contains("No user exists with the provided email "
                          "address; please try again."))
    {
        _popup.setWindowTitle(tr("Invalid username"));
        _popup.setIcon(QMessageBox::Warning);
        _popup.setText(
            tr("Account %1 is invalid; please try again.").arg(_user));
        _popup.exec();
    }
}

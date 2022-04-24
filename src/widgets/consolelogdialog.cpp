#include "consolelogdialog.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QDialog>

#include "ui_consolelogdialog.h"
WARNINGS_ENABLE

#include "LogEntry.h"
#include "TTextView.h"

ConsoleLogDialog::ConsoleLogDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::ConsoleLogDialog)
{
    // Ui initialization
    _ui->setupUi(this);

    // Connect the Ok button
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this,
            &QDialog::accept);
}

ConsoleLogDialog::~ConsoleLogDialog()
{
    delete _ui;
}

void ConsoleLogDialog::appendLogString(const QString &text)
{
    _ui->log->appendLog(LogEntry{QDateTime::currentDateTime(), text});
}

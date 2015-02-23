#include "restoredialog.h"
#include "ui_restoredialog.h"

#include <QStandardPaths>

RestoreDialog::RestoreDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::RestoreDialog)
{
    _ui->setupUi(this);
    _ui->optionsGroupBox->hide();
    _ui->chdirLineEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    _ui->chdirLineEdit->hide();
}

RestoreDialog::~RestoreDialog()
{
    delete _ui;
}

void RestoreDialog::on_pushButton_2_clicked()
{
    reject();
}

void RestoreDialog::on_pushButton_clicked()
{
    accept();
}

void RestoreDialog::on_displayOptionsCheckBox_toggled(bool checked)
{
    _ui->optionsGroupBox->setVisible(checked);
}

void RestoreDialog::on_presetPathsCheckBox_toggled(bool checked)
{
    _ui->chdirLineEdit->setVisible(!checked);
}

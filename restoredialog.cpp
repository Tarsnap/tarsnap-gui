#include "restoredialog.h"
#include "ui_restoredialog.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>

RestoreDialog::RestoreDialog(ArchivePtr archive, QWidget *parent) :
    QDialog(parent), _ui(new Ui::RestoreDialog), _archive(archive)
{
    _ui->setupUi(this);
    _ui->optionsGroupBox->hide();
    _ui->infoLabel->setText(tr("Restore archive <b>%1</b> contents to original locations? "
                               "Any existing data will not be replaced, expand <i>Display options</i> to modify the defaults.")
                            .arg(archive->name));
    QSettings settings;
    _ui->chdirLineEdit->setText(settings.value("app/downloads_dir", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString());
    _ui->chdirLineEdit->hide();
    _ui->browseButton->hide();
    _ui->preservePathsCheckBox->setChecked(settings.value("tarsnap/preserve_pathnames", true).toBool());
}

RestoreDialog::~RestoreDialog()
{
    delete _ui;
}

ArchiveRestoreOptions RestoreDialog::getOptions()
{
    ArchiveRestoreOptions options;
    options.preservePaths  = _ui->preservePathsCheckBox->isChecked();
    options.overwriteFiles = _ui->overwriteCheckBox->isChecked();
    options.keepNewerFiles = _ui->keepNewerCheckBox->isChecked();
    options.chdir = _ui->chdirLineEdit->text();
    return options;
}

void RestoreDialog::on_cancelButton_clicked()
{
    reject();
}

void RestoreDialog::on_restoreButton_clicked()
{
    accept();
}

void RestoreDialog::on_displayOptionsCheckBox_toggled(bool checked)
{
    _ui->optionsGroupBox->setVisible(checked);
    adjustSize();
}

void RestoreDialog::on_preservePathsCheckBox_toggled(bool checked)
{
    _ui->chdirLineEdit->setVisible(!checked);
    _ui->browseButton->setVisible(!checked);
    adjustSize();
}

void RestoreDialog::on_browseButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Directory to restore to"),
                                                     QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    if(!path.isEmpty())
        _ui->chdirLineEdit->setText(path);
}

void RestoreDialog::on_overwriteCheckBox_toggled(bool checked)
{
    _ui->keepNewerCheckBox->setEnabled(checked);
}

void RestoreDialog::on_chdirLineEdit_editingFinished()
{
    QFileInfo file(_ui->chdirLineEdit->text());
    if(file.exists() && file.isDir() && file.isWritable())
        _ui->chdirLineEdit->setStyleSheet("QLineEdit{color:black;}");
    else
        _ui->chdirLineEdit->setStyleSheet("QLineEdit{color:red;}");
}

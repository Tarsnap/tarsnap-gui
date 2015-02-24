#include "restoredialog.h"
#include "ui_restoredialog.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QSettings>

RestoreDialog::RestoreDialog(ArchivePtr archive, QWidget *parent) :
    QDialog(parent), _ui(new Ui::RestoreDialog), _archive(archive)
{
    _ui->setupUi(this);
    _ui->optionsGroupBox->hide();
    _ui->infoLabel->setText(tr("Restore archive <b>%1</b> contents to original locations? "
                               "Any existing data will not be replaced, expand <i>Display options</i> to modify the defaults.")
                            .arg(archive->name));
    _ui->chdirLineEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    _ui->chdirLineEdit->hide();
    _ui->browseButton->hide();
    QSettings settings;
    _ui->preservePathsCheckBox->setChecked(settings.value("tarsnap/preserve_pathnames", true).toBool());
}

RestoreDialog::~RestoreDialog()
{
    delete _ui;
}

ArchiveRestoreOptions RestoreDialog::getOptions()
{
    ArchiveRestoreOptions options;
    options.preservePaths = _ui->preservePathsCheckBox->isChecked();
    options.overwriteFiles = _ui->overwriteCheckBox->isChecked();
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
}

void RestoreDialog::on_preservePathsCheckBox_toggled(bool checked)
{
    _ui->chdirLineEdit->setVisible(!checked);
    _ui->browseButton->setVisible(!checked);
}

void RestoreDialog::on_browseButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Directory to restore to"),
                                                     QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    if(!path.isEmpty())
        _ui->chdirLineEdit->setText(path);
}

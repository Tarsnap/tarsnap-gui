#include "restoredialog.h"
#include "ui_restoredialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

RestoreDialog::RestoreDialog(ArchivePtr archive, QWidget *parent)
    : QDialog(parent), _ui(new Ui::RestoreDialog), _archive(archive)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) &
                   ~Qt::WindowMaximizeButtonHint);

    _ui->infoLabel->setText(_ui->infoLabel->text().arg(archive->name()));
    QSettings settings;
    QString   downDir = settings.value("app/downloads_dir",
                                       QStandardPaths::writableLocation(
                                           QStandardPaths::DownloadLocation))
                          .toString();
    _ui->dirLineEdit->setText(downDir);
    _ui->dirLineEdit->hide();
    _ui->browseButton->hide();
    if(settings.value("tarsnap/preserve_pathnames", true).toBool())
        _ui->preservePathsRadioButton->setChecked(true);
    else
        _ui->chdirRadioButton->setChecked(true);
}

RestoreDialog::~RestoreDialog()
{
    delete _ui;
}

ArchiveRestoreOptions RestoreDialog::getOptions()
{
    ArchiveRestoreOptions options;
    options.preservePaths   = _ui->preservePathsRadioButton->isChecked();
    options.chdir           = _ui->chdirRadioButton->isChecked();
    options.overwriteFiles  = _ui->overwriteCheckBox->isChecked();
    options.keepNewerFiles  = _ui->keepNewerCheckBox->isChecked();
    options.downloadArchive = _ui->downloadArchiveRadioButton->isChecked();
    options.dir             = _ui->dirLineEdit->text();
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

void RestoreDialog::on_chdirRadioButton_toggled(bool checked)
{
    _ui->dirLineEdit->setVisible(checked);
    _ui->browseButton->setVisible(checked);
    _ui->overwriteCheckBox->setVisible(checked);
    _ui->keepNewerCheckBox->setVisible(checked);
    adjustSize();
}

void RestoreDialog::on_browseButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(
        this, tr("Directory to restore to"),
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    if(!path.isEmpty())
        _ui->dirLineEdit->setText(path);
}

void RestoreDialog::on_overwriteCheckBox_toggled(bool checked)
{
    _ui->keepNewerCheckBox->setEnabled(checked);
}

void RestoreDialog::on_dirLineEdit_editingFinished()
{
    QFileInfo file(_ui->dirLineEdit->text());
    if(file.exists() && file.isDir() && file.isWritable())
        _ui->dirLineEdit->setStyleSheet("QLineEdit{color:black;}");
    else
        _ui->dirLineEdit->setStyleSheet("QLineEdit{color:red;}");
}

void RestoreDialog::on_downloadArchiveRadioButton_toggled(bool checked)
{
    _ui->dirLineEdit->setVisible(checked);
    _ui->browseButton->setVisible(checked);
    adjustSize();
}

void RestoreDialog::on_preservePathsRadioButton_toggled(bool checked)
{
    _ui->overwriteCheckBox->setVisible(checked);
    _ui->keepNewerCheckBox->setVisible(checked);
    adjustSize();
}

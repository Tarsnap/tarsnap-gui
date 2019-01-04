#include "restoredialog.h"
#include "utils.h"

#include "ui_restoredialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <TSettings.h>

RestoreDialog::RestoreDialog(QWidget *parent, ArchivePtr archive,
                             QStringList files)
    : QDialog(parent),
      _ui(new Ui::RestoreDialog),
      _archive(archive),
      _files(files)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    _ui->setupUi(this);

    TSettings settings;
    _downDir = settings.value("app/downloads_dir", DEFAULT_DOWNLOADS).toString();
    _ui->baseDirLineEdit->setText(_downDir);
    _ui->baseDirLineEdit->hide();
    _ui->changeDirButton->hide();
    QString fileName(_archive->name() + ".tar");
    // Replace chars that are problematic on common file systems but are allowed
    // in tarsnap archive names
    fileName = fileName.replace(QChar(':'), QChar('-'))
                   .replace(QChar('/'), QChar('-'))
                   .replace(QChar('\\'), QChar('-'));
    QFileInfo archiveFile(QDir(_downDir), fileName);
    archiveFile.makeAbsolute();
    _ui->archiveLineEdit->setText(archiveFile.absoluteFilePath());
    _ui->archiveLineEdit->hide();
    _ui->changeArchiveButton->hide();

    connect(_ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(_ui->restoreButton, &QPushButton::clicked, this, [&]() {
        if(validate())
            accept();
    });
    connect(_ui->changeDirButton, &QPushButton::clicked, this,
            &RestoreDialog::changeDir);
    connect(_ui->changeArchiveButton, &QPushButton::clicked, this,
            &RestoreDialog::changeArchive);
    connect(_ui->optionRestoreRadio, &QRadioButton::toggled, this,
            &RestoreDialog::optionRestoreToggled);
    connect(_ui->optionBaseDirRadio, &QRadioButton::toggled, this,
            &RestoreDialog::optionBaseDirToggled);
    connect(_ui->optionTarArchiveRadio, &QRadioButton::toggled, this,
            &RestoreDialog::optionTarArchiveToggled);
    connect(_ui->overwriteCheckBox, &QCheckBox::toggled, this, [&](bool checked) {
        _ui->keepNewerCheckBox->setChecked(checked);
        _ui->keepNewerCheckBox->setEnabled(checked);
    });
    connect(_ui->baseDirLineEdit, &QLineEdit::textChanged, this,
            &RestoreDialog::validate);
    connect(_ui->archiveLineEdit, &QLineEdit::textChanged, this,
            &RestoreDialog::validate);

    bool canRestore = _archive->hasPreservePaths();
    displayRestoreOption(canRestore);
    _ui->optionRestoreRadio->setChecked(canRestore);
    _ui->optionBaseDirRadio->setChecked(!canRestore);
    if(!_files.isEmpty())
    {
        _ui->filesListWidget->addItems(_files);
    }
    else
    {
        if(_archive->contents().isEmpty())
        {
            _ui->filesListWidget->hide();
            adjustSize();
        }
        else
        {
            _ui->filesListWidget->addItems(
                _archive->contents().split(QChar('\n')));
            _ui->filesListWidget->show();
            adjustSize();
        }
    }
}

RestoreDialog::~RestoreDialog()
{
    delete _ui;
}

ArchiveRestoreOptions RestoreDialog::getOptions()
{
    ArchiveRestoreOptions options;
    options.optionRestore    = _ui->optionRestoreRadio->isChecked();
    options.optionRestoreDir = _ui->optionBaseDirRadio->isChecked();
    options.optionTarArchive = _ui->optionTarArchiveRadio->isChecked();
    options.overwriteFiles   = _ui->overwriteCheckBox->isChecked();
    options.keepNewerFiles   = _ui->keepNewerCheckBox->isChecked();
    options.preservePerms    = _ui->preservePermCheckBox->isChecked();
    options.files            = _files;
    if(options.optionRestoreDir)
        options.path = _ui->baseDirLineEdit->text();
    else if(options.optionTarArchive)
        options.path = _ui->archiveLineEdit->text();
    return options;
}

void RestoreDialog::displayRestoreOption(bool display)
{
    _ui->optionRestoreRadio->setVisible(display);
    adjustSize();
}

void RestoreDialog::displayTarOption(bool display)
{
    _ui->optionTarArchiveRadio->setVisible(display);
    adjustSize();
}

void RestoreDialog::optionBaseDirToggled(bool checked)
{
    _ui->infoLabel->setText(tr("Restore from archive <b>%1</b> to specified"
                               " directory? Any existing files will not be"
                               " replaced by default. Use the options below to"
                               " modify this behavior:")
                                .arg(_archive->name()));
    _ui->baseDirLineEdit->setVisible(checked);
    _ui->changeDirButton->setVisible(checked);
    _ui->overwriteCheckBox->setVisible(checked);
    _ui->keepNewerCheckBox->setVisible(checked);
    _ui->preservePermCheckBox->setVisible(checked);
    adjustSize();
}

void RestoreDialog::optionTarArchiveToggled(bool checked)
{
    _ui->infoLabel->setText(tr("Download archive <b>%1</b> contents as an"
                               " uncompressed TAR archive?")
                                .arg(_archive->name()));
    _ui->archiveLineEdit->setVisible(checked);
    _ui->changeArchiveButton->setVisible(checked);
    adjustSize();
}

void RestoreDialog::optionRestoreToggled(bool checked)
{
    _ui->infoLabel->setText(tr("Restore from archive <b>%1</b> to original"
                               " locations? Any existing files will not be"
                               " replaced by default. Use the options below to"
                               " modify this behavior:")
                                .arg(_archive->name()));
    _ui->overwriteCheckBox->setVisible(checked);
    _ui->keepNewerCheckBox->setVisible(checked);
    _ui->preservePermCheckBox->setVisible(checked);
    adjustSize();
}

void RestoreDialog::changeDir()
{
    QString path =
        QFileDialog::getExistingDirectory(this, tr("Directory to restore to"),
                                          _downDir);
    if(!path.isEmpty())
        _ui->baseDirLineEdit->setText(path);
}

void RestoreDialog::changeArchive()
{
    QString path =
        QFileDialog::getSaveFileName(this, tr("Select tar archive file"),
                                     _ui->archiveLineEdit->text(),
                                     tr("Tar archives (*.tar)"));
    if(!path.isEmpty())
        _ui->archiveLineEdit->setText(path);
}

bool RestoreDialog::validate()
{
    bool valid = true;
    if(_ui->optionBaseDirRadio->isChecked())
    {
        QFileInfo dir(_ui->baseDirLineEdit->text());
        if(dir.exists() && dir.isDir())
        {
            _ui->baseDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
            _ui->baseDirLineEdit->setToolTip(
                tr("Set base directory to extract archive contents to"));
        }
        else
        {
            _ui->baseDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
            _ui->baseDirLineEdit->setToolTip(
                tr("Invalid base directory, please choose a different one"));
            valid = false;
        }
    }
    else if(_ui->optionTarArchiveRadio->isChecked())
    {
        QFileInfo archive(_ui->archiveLineEdit->text());
        if(archive.exists())
        {
            _ui->archiveLineEdit->setStyleSheet("QLineEdit{color:red;}");
            _ui->archiveLineEdit->setToolTip(
                tr("File exists, please choose a different file name"));
            valid = false;
        }
        else
        {
            _ui->archiveLineEdit->setStyleSheet("QLineEdit{color:black;}");
            _ui->archiveLineEdit->setToolTip(tr("Set archive file name"));
        }
    }
    return valid;
}

ArchivePtr RestoreDialog::archive() const
{
    return _archive;
}

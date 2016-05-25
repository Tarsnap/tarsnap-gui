#include "restoredialog.h"
#include "utils.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

RestoreDialog::RestoreDialog(ArchivePtr archive, QWidget *parent)
    : QDialog(parent), _archive(archive)
{
    _ui.setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) &
                   ~Qt::WindowMaximizeButtonHint);

    _ui.infoLabel->setText(_ui.infoLabel->text().arg(archive->name()));
    QSettings settings;
    _downDir = settings.value("app/downloads_dir", DOWNLOADS).toString();
    _ui.baseDirLineEdit->setText(_downDir);
    _ui.baseDirLineEdit->hide();
    _ui.changeDirButton->hide();
    QString fileName(_archive->name() + ".tar");
    // Replace chars that are problematic on common file systems but are allowed
    // in tarsnap archive names
    fileName = fileName.replace(QChar(':'), QChar('-'))
                       .replace(QChar('/'), QChar('-'))
                       .replace(QChar('\\'), QChar('-'));
    QFileInfo archiveFile(QDir(_downDir), fileName);
    archiveFile.makeAbsolute();
    _ui.archiveLineEdit->setText(archiveFile.absoluteFilePath());
    _ui.archiveLineEdit->hide();
    _ui.changeArchiveButton->hide();

    connect(_ui.cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(_ui.restoreButton, &QPushButton::clicked, this, [&]() {
        if(validate())
            accept();
    });
    connect(_ui.changeDirButton, &QPushButton::clicked, this,
            &RestoreDialog::changeDir);
    connect(_ui.changeArchiveButton, &QPushButton::clicked, this,
            &RestoreDialog::changeArchive);
    connect(_ui.optionRestoreRadio, &QRadioButton::toggled, this,
            &RestoreDialog::optionRestoreToggled);
    connect(_ui.optionBaseDirRadio, &QRadioButton::toggled, this,
            &RestoreDialog::optionBaseDirToggled);
    connect(_ui.optionDownArchiveRadio, &QRadioButton::toggled, this,
            &RestoreDialog::optionDownArchiveToggled);
    connect(_ui.overwriteCheckBox, &QCheckBox::toggled, this, [&](bool checked) {
        _ui.keepNewerCheckBox->setChecked(checked);
        _ui.keepNewerCheckBox->setEnabled(checked);
    });
    connect(_ui.baseDirLineEdit, &QLineEdit::textChanged, this,
            &RestoreDialog::validate);
    connect(_ui.archiveLineEdit, &QLineEdit::textChanged, this,
            &RestoreDialog::validate);

    if(settings.value("tarsnap/preserve_pathnames", true).toBool())
        _ui.optionRestoreRadio->setChecked(true);
    else
        _ui.optionBaseDirRadio->setChecked(true);

    adjustSize();
}

RestoreDialog::~RestoreDialog()
{
}

ArchiveRestoreOptions RestoreDialog::getOptions()
{
    ArchiveRestoreOptions options;
    options.optionRestore     = _ui.optionRestoreRadio->isChecked();
    options.optionRestoreDir  = _ui.optionBaseDirRadio->isChecked();
    options.overwriteFiles    = _ui.overwriteCheckBox->isChecked();
    options.keepNewerFiles    = _ui.keepNewerCheckBox->isChecked();
    options.optionDownArchive = _ui.optionDownArchiveRadio->isChecked();
    if(options.optionRestoreDir)
        options.path = _ui.baseDirLineEdit->text();
    else if(options.optionDownArchive)
        options.path = _ui.archiveLineEdit->text();
    return options;
}

void RestoreDialog::optionBaseDirToggled(bool checked)
{
    _ui.baseDirLineEdit->setVisible(checked);
    _ui.changeDirButton->setVisible(checked);
    _ui.overwriteCheckBox->setVisible(checked);
    _ui.keepNewerCheckBox->setVisible(checked);
    adjustSize();
}

void RestoreDialog::optionDownArchiveToggled(bool checked)
{
    _ui.archiveLineEdit->setVisible(checked);
    _ui.changeArchiveButton->setVisible(checked);
    adjustSize();
}

void RestoreDialog::optionRestoreToggled(bool checked)
{
    _ui.overwriteCheckBox->setVisible(checked);
    _ui.keepNewerCheckBox->setVisible(checked);
    adjustSize();
}

void RestoreDialog::changeDir()
{
    QString path =
        QFileDialog::getExistingDirectory(this, tr("Directory to restore to"),
                                          _downDir);
    if(!path.isEmpty())
        _ui.baseDirLineEdit->setText(path);
}

void RestoreDialog::changeArchive()
{
    QString path =
        QFileDialog::getSaveFileName(this, tr("Select tar archive file"),
                                     _ui.archiveLineEdit->text(),
                                     tr("Tar archives (*.tar)"));
    if(!path.isEmpty())
        _ui.archiveLineEdit->setText(path);
}

bool RestoreDialog::validate()
{
    bool valid = true;
    if(_ui.optionBaseDirRadio->isChecked())
    {
        QFileInfo dir(_ui.baseDirLineEdit->text());
        if(dir.exists() && dir.isDir() && dir.isWritable())
        {
            _ui.baseDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
            _ui.baseDirLineEdit->setToolTip(
                tr("Set base directory to extract archive contents to"));
        }
        else
        {
            _ui.baseDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
            _ui.baseDirLineEdit->setToolTip(
                tr("Invalid base directory. Please choose a different one."));
            valid = false;
        }
    }
    else if(_ui.optionDownArchiveRadio->isChecked())
    {
        QFileInfo archive(_ui.archiveLineEdit->text());
        if(archive.exists())
        {
            _ui.archiveLineEdit->setStyleSheet("QLineEdit{color:red;}");
            _ui.archiveLineEdit->setToolTip(
                tr("File exists. Please choose a different file name."));
            valid = false;
        }
        else
        {
            _ui.archiveLineEdit->setStyleSheet("QLineEdit{color:black;}");
            _ui.archiveLineEdit->setToolTip(tr("Set archive file name"));
        }
    }
    return valid;
}

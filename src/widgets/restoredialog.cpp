#include "restoredialog.h"
#include "ui_restoredialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
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
    QString   _downDir = settings.value("app/downloads_dir",
                                        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
                         .toString();
    _ui->baseDirLineEdit->setText(_downDir);
    _ui->baseDirLineEdit->hide();
    _ui->changeDirButton->hide();
    _ui->changeArchiveButton->hide();
    _ui->archiveLineEdit->hide();

    connect(_ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(_ui->restoreButton, &QPushButton::clicked, this, [&]()
    { if(validate()) accept();});
    connect(_ui->changeDirButton, &QPushButton::clicked, this, &RestoreDialog::changeDir);
    connect(_ui->changeArchiveButton, &QPushButton::clicked, this, &RestoreDialog::changeArchive);
    connect(_ui->preservePathsRadioButton, &QRadioButton::toggled, this, &RestoreDialog::preservePathsToggled);
    connect(_ui->baseDirRadioButton, &QRadioButton::toggled, this, &RestoreDialog::baseDirToggled);
    connect(_ui->getArchiveRadioButton, &QRadioButton::toggled, this, &RestoreDialog::getArchiveToggled);
    connect(_ui->overwriteCheckBox, &QCheckBox::toggled, this, [&](bool checked)
    {_ui->keepNewerCheckBox->setEnabled(checked);});
    connect(_ui->baseDirLineEdit, &QLineEdit::textChanged, this, &RestoreDialog::validate);
    connect(_ui->archiveLineEdit, &QLineEdit::textChanged, this, &RestoreDialog::validate);

    if(settings.value("tarsnap/preserve_pathnames", true).toBool())
        _ui->preservePathsRadioButton->setChecked(true);
    else
        _ui->baseDirRadioButton->setChecked(true);
}

RestoreDialog::~RestoreDialog()
{
    delete _ui;
}

ArchiveRestoreOptions RestoreDialog::getOptions()
{
    ArchiveRestoreOptions options;
    options.preservePaths    = _ui->preservePathsRadioButton->isChecked();
    options.baseDir          = _ui->baseDirRadioButton->isChecked();
    options.overwriteFiles   = _ui->overwriteCheckBox->isChecked();
    options.keepNewerFiles   = _ui->keepNewerCheckBox->isChecked();
    options.getArchive       = _ui->getArchiveRadioButton->isChecked();
    if(options.baseDir)
        options.path = _ui->baseDirLineEdit->text();
    else if(options.getArchive)
        options.path = _ui->archiveLineEdit->text();
    return options;
}

void RestoreDialog::baseDirToggled(bool checked)
{
    _ui->baseDirLineEdit->setVisible(checked);
    _ui->changeDirButton->setVisible(checked);
    _ui->overwriteCheckBox->setVisible(checked);
    _ui->keepNewerCheckBox->setVisible(checked);
    adjustSize();
}

void RestoreDialog::changeDir()
{
    QString path = QFileDialog::getExistingDirectory(this,
                   tr("Directory to restore to"), _downDir);
    _ui->baseDirLineEdit->setText(path);
}

void RestoreDialog::changeArchive()
{
    QFileInfo archiveFile(QDir(_downDir), _archive->name() + ".tar");
    archiveFile.makeAbsolute();
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Select tar archive file"),
                                                archiveFile.absoluteFilePath(),
                                                tr("Tar archives (*.tar)"));
    _ui->archiveLineEdit->setText(path);
}

bool RestoreDialog::validate()
{
    bool valid = true;
    if(_ui->baseDirRadioButton->isChecked())
    {
        QFileInfo dir(_ui->baseDirLineEdit->text());
        if(dir.exists() && dir.isDir() && dir.isWritable())
        {
            _ui->baseDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
        }
        else
        {
            _ui->baseDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
            valid = false;
        }
    }
    else if(_ui->getArchiveRadioButton->isChecked())
    {
        QFileInfo archive(_ui->archiveLineEdit->text());
        if(archive.exists())
        {
            _ui->archiveLineEdit->setStyleSheet("QLineEdit{color:red;}");
            valid = false;
        }
        else
        {
            _ui->archiveLineEdit->setStyleSheet("QLineEdit{color:black;}");
        }
    }
    return valid;
}

void RestoreDialog::getArchiveToggled(bool checked)
{
    _ui->archiveLineEdit->setVisible(checked);
    _ui->changeArchiveButton->setVisible(checked);
    adjustSize();
}

void RestoreDialog::preservePathsToggled(bool checked)
{
    _ui->overwriteCheckBox->setVisible(checked);
    _ui->keepNewerCheckBox->setVisible(checked);
    adjustSize();
}

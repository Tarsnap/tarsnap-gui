#include "setupdialog.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

// This is used for QHostInfo::localHostName().  It could be replaced with
// QSysInfo::machineHostName() to remove the dependency on the Qt network
// library, but only if we require Qt 5.6+.
#include <QHostInfo>

#include "ui_setupdialog.h"
WARNINGS_ENABLE

#include "debug.h"
#include "tasks-defs.h"
#include "utils.h"

#include <TSettings.h>

SetupDialog::SetupDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::SetupDialog)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint)
                   & ~Qt::WindowMaximizeButtonHint);

    // This should be done before connecting objects to
    // validateAdvancedSetupPage() to avoid calling that function
    // unnecessarily.
    initCLIPage();

    // All pages
    connect(_ui->backButton, &QPushButton::clicked, this,
            &SetupDialog::backButtonClicked);
    connect(_ui->nextButton, &QPushButton::clicked, this,
            &SetupDialog::nextButtonClicked);
    connect(_ui->welcomePageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);
    connect(_ui->advancedPageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);
    connect(_ui->registerPageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);
    connect(_ui->donePageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);

    connect(_ui->wizardStackedWidget, &QStackedWidget::currentChanged, this,
            &SetupDialog::wizardPageChanged);

    // Advanced setup page
    connect(_ui->advancedCLIButton, &QPushButton::toggled,
            _ui->advancedCLIWidget, &QWidget::setVisible);
    connect(_ui->tarsnapPathBrowseButton, &QPushButton::clicked, this,
            &SetupDialog::showTarsnapPathBrowse);
    connect(_ui->tarsnapPathLineEdit, &QLineEdit::textChanged, this,
            &SetupDialog::validateAdvancedSetupPage);
    connect(_ui->tarsnapCacheBrowseButton, &QPushButton::clicked, this,
            &SetupDialog::showTarsnapCacheBrowse);
    connect(_ui->tarsnapCacheLineEdit, &QLineEdit::textChanged, this,
            &SetupDialog::validateAdvancedSetupPage);
    connect(_ui->appDataBrowseButton, &QPushButton::clicked, this,
            &SetupDialog::showAppDataBrowse);
    connect(_ui->appDataPathLineEdit, &QLineEdit::textChanged, this,
            &SetupDialog::validateAdvancedSetupPage);

    // Register page
    connect(_ui->restoreNoButton, &QPushButton::clicked, this,
            &SetupDialog::restoreNo);
    connect(_ui->restoreYesButton, &QPushButton::clicked, this,
            &SetupDialog::restoreYes);
    connect(_ui->tarsnapUserLineEdit, &QLineEdit::textChanged, this,
            &SetupDialog::validateRegisterPage);
    connect(_ui->tarsnapPasswordLineEdit, &QLineEdit::textChanged, this,
            &SetupDialog::validateRegisterPage);
    connect(_ui->machineNameLineEdit, &QLineEdit::textChanged, this,
            &SetupDialog::validateRegisterPage);
    connect(_ui->machineKeyCombo, &QComboBox::currentTextChanged, this,
            &SetupDialog::validateRegisterPage);
    connect(_ui->browseKeyButton, &QPushButton::clicked, this,
            &SetupDialog::registerHaveKeyBrowse);

    // Initial state
    _ui->wizardStackedWidget->setCurrentWidget(_ui->welcomePage);
    wizardPageChanged(0);
}

SetupDialog::~SetupDialog()
{
    delete _ui;
}

void SetupDialog::initCLIPage()
{
    QString tarsnapDir;

    tarsnapDir = Utils::findTarsnapClientInPath("", true);
    _ui->tarsnapPathLineEdit->setText(tarsnapDir);
    _ui->machineNameLineEdit->setText(QHostInfo::localHostName());

    _appDataDir = QStandardPaths::writableLocation(APPDATA);
    QDir keysDir(_appDataDir);
    if(!keysDir.exists())
        keysDir.mkpath(_appDataDir);
    _ui->appDataPathLineEdit->setText(_appDataDir);

    // find existing keys
    for(const QFileInfo &file : Utils::findKeysInPath(_appDataDir))
        _ui->machineKeyCombo->addItem(file.canonicalFilePath());

    _tarsnapCacheDir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(_tarsnapCacheDir);
    if(!cacheDir.exists())
        cacheDir.mkpath(_tarsnapCacheDir);
    _ui->tarsnapCacheLineEdit->setText(_tarsnapCacheDir);

    _ui->advancedCLIWidget->hide();
}

void SetupDialog::wizardPageChanged(int)
{
    // Values which might be overwritten below.
    _ui->backButton->setText(tr("Back"));
    _ui->nextButton->setText(tr("Next"));
    _ui->nextButton->setEnabled(true);

    if(_ui->wizardStackedWidget->currentWidget() == _ui->welcomePage)
    {
        _ui->welcomePageRadioButton->setChecked(true);
        _ui->titleLabel->setText(tr("Setup wizard"));
        _ui->backButton->setText(tr("Skip wizard"));
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->advancedPage)
    {
        _ui->advancedPageRadioButton->setChecked(true);
        _ui->titleLabel->setText(tr("Command-line utilities"));
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
    {
        _ui->registerPageRadioButton->setChecked(true);
        _ui->titleLabel->setText(tr("Register with server"));
        _ui->nextButton->setText(tr("Register machine"));
        if(_ui->machineKeyCombo->count() > 0)
        {
            _ui->restoreYesButton->setChecked(true);
            restoreYes();
        }
        else
        {
            _ui->restoreNoButton->setChecked(true);
            restoreNo();
        }
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->donePage)
    {
        _ui->donePageRadioButton->setChecked(true);
        _ui->titleLabel->setText(tr("Setup complete!"));
        _ui->nextButton->setText(tr("Start using Tarsnap"));
    }
}

void SetupDialog::backButtonClicked()
{
    int nextIndex = _ui->wizardStackedWidget->currentIndex() - 1;
    if(nextIndex < 0)
        commitSettings(true);
    else
        _ui->wizardStackedWidget->setCurrentIndex(nextIndex);
}

void SetupDialog::nextButtonClicked()
{
    if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
        registerMachine();
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->donePage)
        commitSettings(false);
    else
        setNextPage();
}

void SetupDialog::skipToPage()
{
    if(sender() == _ui->welcomePageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->welcomePage);
    else if(sender() == _ui->advancedPageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->advancedPage);
    else if(sender() == _ui->registerPageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->registerPage);
    else if(sender() == _ui->donePageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->donePage);
}

void SetupDialog::setNextPage()
{
    if(_ui->wizardStackedWidget->currentWidget() == _ui->welcomePage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->advancedPage);
        _ui->advancedPageRadioButton->setEnabled(true);
        // Disable this until we know the version number.
        _ui->nextButton->setEnabled(false);
        bool advancedOk = validateAdvancedSetupPage();
        _ui->advancedCLIButton->setChecked(!advancedOk);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->advancedPage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->registerPage);
        _ui->registerPageRadioButton->setEnabled(true);
        if(validateRegisterPage())
            _ui->nextButton->setFocus();
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->donePage);
        _ui->donePageRadioButton->setEnabled(true);
    }
}

void SetupDialog::showTarsnapPathBrowse()
{
    QString tarsnapPath =
        QFileDialog::getExistingDirectory(this, tr("Find Tarsnap client"), "");
    _ui->tarsnapPathLineEdit->setText(tarsnapPath);
}

void SetupDialog::showTarsnapCacheBrowse()
{
    QString tarsnapCacheDir =
        QFileDialog::getExistingDirectory(this, tr("Tarsnap cache location"),
                                          _tarsnapCacheDir);
    _ui->tarsnapCacheLineEdit->setText(tarsnapCacheDir);
}

void SetupDialog::showAppDataBrowse()
{
    QString appDataDir =
        QFileDialog::getExistingDirectory(this, tr("App data location"), "");
    _ui->appDataPathLineEdit->setText(appDataDir);
}

bool SetupDialog::validateAdvancedSetupPage()
{
    QString tarsnapDir;

    bool result = true;

    _appDataDir = Utils::validateAppDataDir(_ui->appDataPathLineEdit->text());
    if(_appDataDir.isEmpty())
    {
        _ui->advancedValidationLabel->setText(tr("Invalid App data directory "
                                                 "set."));
        result = false;
    }

    _tarsnapCacheDir =
        Utils::validateTarsnapCache(_ui->tarsnapCacheLineEdit->text());
    if(result && _tarsnapCacheDir.isEmpty())
    {
        _ui->advancedValidationLabel->setText(
            tr("Invalid Tarsnap cache directory"
               " set."));
        result = false;
    }

    tarsnapDir =
        Utils::findTarsnapClientInPath(_ui->tarsnapPathLineEdit->text(), true);
    if(result && tarsnapDir.isEmpty())
    {
        _ui->advancedValidationLabel->setText(
            tr("Tarsnap utilities not found. Visit "
               "<a href=\"https://tarsnap.com\">tarsnap.com</a> "
               "for help with acquiring them."));
        result = false;
    }
    else if(result)
    {
        TSettings settings;
        settings.setValue("tarsnap/path", tarsnapDir);
        // Wipe previous version number before asking for a new one.
        settings.setValue("tarsnap/version", "");
        emit tarsnapVersionRequested();
    }

    // If `results` is true, then we might still be waiting for
    // the result of tarsnapVersion, so we can't enable this yet.
    if(!result)
        _ui->nextButton->setEnabled(false);

    return result;
}

void SetupDialog::restoreNo()
{
    _ui->registerKeyStackedWidget->setCurrentWidget(_ui->keyNoPage);
    // Share machineNameLineEdit in both pages of the keyStackedWidget
    _ui->gridKeyNoLayout->addWidget(_ui->machineNameLineEdit, 1, 1);
    _ui->statusLabel->clear();
    if(validateRegisterPage())
        _ui->nextButton->setFocus();
}

void SetupDialog::restoreYes()
{
    _ui->registerKeyStackedWidget->setCurrentWidget(_ui->keyYesPage);
    // Share machineNameLineEdit in both pages of the keyStackedWidget
    _ui->gridKeyYesLayout->addWidget(_ui->machineNameLineEdit, 1, 1);
    _ui->statusLabel->clear();
    if(validateRegisterPage())
        _ui->nextButton->setFocus();
}

bool SetupDialog::validateRegisterPage()
{
    bool result = false;
    if(_ui->restoreYesButton->isChecked())
    {
        // user specified key
        QFileInfo machineKeyFile(_ui->machineKeyCombo->currentText());
        if(!_ui->machineNameLineEdit->text().isEmpty()
           && machineKeyFile.exists() && machineKeyFile.isFile()
           && machineKeyFile.isReadable())
        {
            result = true;
        }
    }
    else
    {
        if(!_ui->tarsnapUserLineEdit->text().isEmpty()
           && !_ui->tarsnapPasswordLineEdit->text().isEmpty()
           && !_ui->machineNameLineEdit->text().isEmpty())
        {
            result = true;
        }
    }

    _ui->nextButton->setEnabled(result);
    return result;
}

void SetupDialog::registerHaveKeyBrowse()
{
    QString keyFilter = tr("Tarsnap key files (*.key *.keys)");
    QString existingMachineKey =
        QFileDialog::getOpenFileName(this,
                                     tr("Browse for existing machine key"), "",
                                     tr("All files (*);;") + keyFilter,
                                     &keyFilter);
    if(!existingMachineKey.isEmpty())
        _ui->machineKeyCombo->setCurrentText(existingMachineKey);
}

void SetupDialog::registerMachine()
{
    TSettings settings;

    // Sanity check app data dir.
    if(_appDataDir.isEmpty())
    {
        // We should never get here, but handle the error anyway
        _ui->statusLabel->setText("No app data dir set");
        _ui->statusLabel->setStyleSheet("#statusLabel { color: darkred; }");
        _ui->nextButton->setEnabled(false);
        return;
    }

    bool useExistingKeyfile = false;
    _ui->nextButton->setEnabled(false);
    _ui->statusLabel->clear();
    _ui->statusLabel->setStyleSheet("");
    if(_ui->restoreYesButton->isChecked())
    {
        useExistingKeyfile = true;
        _ui->statusLabel->setText("Verifying archive integrity...");
        _tarsnapKeyFile = _ui->machineKeyCombo->currentText();
    }
    else
    {
        _ui->statusLabel->setText("Generating keyfile...");
        _tarsnapKeyFile =
            _appDataDir + QDir::separator() + _ui->machineNameLineEdit->text()
            + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")
            + ".key";
    }

    settings.setValue("tarsnap/cache", _tarsnapCacheDir);
    settings.setValue("tarsnap/key", _tarsnapKeyFile);
    settings.setValue("tarsnap/user", _ui->tarsnapUserLineEdit->text());
    settings.setValue("tarsnap/machine", _ui->machineNameLineEdit->text());

    emit registerMachineRequested(_ui->tarsnapPasswordLineEdit->text(),
                                  useExistingKeyfile);
}

void SetupDialog::registerMachineResponse(TaskStatus status, QString reason)
{
    switch(status)
    {
    case TaskStatus::Completed:
        _ui->statusLabel->clear();
        _ui->doneKeyFileNameLabel->setText(
            QString("<a href=\"%1\">%2</a>")
                .arg(QUrl::fromLocalFile(
                         QFileInfo(_tarsnapKeyFile).absolutePath())
                         .toString())
                .arg(_tarsnapKeyFile));
        _ui->nextButton->setEnabled(true);
        setNextPage();
        break;
    case TaskStatus::Failed:
        _ui->statusLabel->setText(reason);
        _ui->statusLabel->setStyleSheet("#statusLabel { color: darkred; }");
        _ui->nextButton->setEnabled(true);
        break;
    default:
        // We shouldn't receive anything else, so ignore it.
        break;
    }
}

void SetupDialog::updateLoadingAnimation(bool idle)
{
    _ui->busyWidget->animate(!idle);
}

void SetupDialog::tarsnapVersionResponse(TaskStatus status,
                                         QString    versionString)
{
    TSettings settings;

    // Sanity check.
    if(versionString.isEmpty())
        status = TaskStatus::Failed;

    // Handle response.
    switch(status)
    {
    case TaskStatus::Completed:
        _ui->advancedValidationLabel->setText(
            tr("Tarsnap CLI version ") + versionString + tr(" detected.  ✔"));
        // Record value
        settings.setValue("tarsnap/version", versionString);
        // Enable progress
        _ui->nextButton->setEnabled(true);
        _ui->nextButton->setFocus();
        break;
    case TaskStatus::VersionTooLow:
        // Don't record the too-low version number.
        _ui->advancedValidationLabel->setText(
            tr("Tarsnap CLI version ") + versionString
            + tr(" too low; must be at least %1").arg(TARSNAP_MIN_VERSION));
        break;
    case TaskStatus::Failed:
        _ui->advancedValidationLabel->setText(
            tr("Error retrieving Tarsnap CLI verison"));
        break;
    default:
        break;
    }
}

void SetupDialog::commitSettings(bool skipped)
{
    TSettings settings;

    settings.setValue("app/wizard_done", true);

    if(!skipped)
    {
        settings.setValue("app/app_data", _appDataDir);
    }
    settings.sync();

    accept();
}

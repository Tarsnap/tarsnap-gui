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

#include <PathLineBrowse.h>
#include <TSettings.h>

SetupDialog::SetupDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::SetupDialog)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint)
                   & ~Qt::WindowMaximizeButtonHint);

    // This should be done before connecting objects to
    // validateCLIPage() to avoid calling that function unnecessarily.
    initCLIPage();
    initRegisterPage();

    // All pages
    connect(_ui->backButton, &QPushButton::clicked, this,
            &SetupDialog::backButtonClicked);
    connect(_ui->nextButton, &QPushButton::clicked, this,
            &SetupDialog::nextButtonClicked);

    connect(_ui->wizardStackedWidget, &QStackedWidget::currentChanged, this,
            &SetupDialog::wizardPageChanged);

    // Advanced setup page
    connect(_ui->cliAdvancedButton, &QPushButton::toggled,
            _ui->cliAdvancedWidget, &QWidget::setVisible);
    connect(_ui->cliPathLineBrowse, &PathLineBrowse::textChanged, this,
            &SetupDialog::validateCLIPage);
    connect(_ui->cachePathLineBrowse, &PathLineBrowse::textChanged, this,
            &SetupDialog::validateCLIPage);
    connect(_ui->appdataPathLineBrowse, &PathLineBrowse::textChanged, this,
            &SetupDialog::validateCLIPage);

    // Register page
    connect(_ui->createKeyfileButton, &QPushButton::clicked, this,
            &SetupDialog::createKeyfile);
    connect(_ui->useExistingKeyfileButton, &QPushButton::clicked, this,
            &SetupDialog::useExistingKeyfile);
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
    QString appDataDir;
    QString tarsnapCacheDir;

    // CLI path
    struct DirMessage cliDirMessage = Utils::findTarsnapClientInPath("", true);
    tarsnapDir                      = cliDirMessage.dirname;
    _ui->cliPathLineBrowse->setText(tarsnapDir);

    // appdata dir
    appDataDir = QStandardPaths::writableLocation(APPDATA);
    // Create directory (if needed)
    QDir keysDir(appDataDir);
    if(!keysDir.exists())
        keysDir.mkpath(appDataDir);
    _ui->appdataPathLineBrowse->setText(appDataDir);

    /// cache dir
    tarsnapCacheDir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(tarsnapCacheDir);
    // Create directory (if needed)
    if(!cacheDir.exists())
        cacheDir.mkpath(tarsnapCacheDir);
    _ui->cachePathLineBrowse->setText(tarsnapCacheDir);

    // We only want to expand the widget if there's a problem
    _ui->cliAdvancedWidget->hide();
}

void SetupDialog::initRegisterPage()
{
    // default machine name
    _ui->machineNameLineEdit->setText(QHostInfo::localHostName());

    // find any existing keys
    QString appDataDir = _ui->appdataPathLineBrowse->text();
    for(const QFileInfo &file : Utils::findKeysInPath(appDataDir))
        _ui->machineKeyCombo->addItem(file.canonicalFilePath());
}

void SetupDialog::wizardPageChanged(int)
{
    // Values which might be overwritten below.
    _ui->backButton->setText(tr("Back"));
    _ui->nextButton->setText(tr("Next"));
    _ui->nextButton->setEnabled(true);

    if(_ui->wizardStackedWidget->currentWidget() == _ui->welcomePage)
    {
        _ui->titleLabel->setText(tr("Setup wizard"));
        _ui->backButton->setText(tr("Skip wizard"));
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->cliPage)
    {
        _ui->titleLabel->setText(tr("Command-line utilities"));
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
    {
        _ui->titleLabel->setText(tr("Register with server"));
        _ui->nextButton->setText(tr("Register machine"));
        if(_ui->machineKeyCombo->count() > 0)
        {
            _ui->useExistingKeyfileButton->setChecked(true);
            useExistingKeyfile();
        }
        else
        {
            _ui->createKeyfileButton->setChecked(true);
            createKeyfile();
        }
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->donePage)
    {
        _ui->titleLabel->setText(tr("Setup complete!"));
        _ui->nextButton->setText(tr("Start using Tarsnap"));
    }
}

#ifdef QT_TESTLIB_LIB
QAbstractButton *SetupDialog::button(enum SetupDialog::Button which)
{
    // These buttons do two things.
    switch(which)
    {
    case SkipButton:
    case BackButton:
        return _ui->backButton;
    case NextButton:
    case FinishButton:
        return _ui->nextButton;
    }
    // Shouldn't reach here.
    return nullptr;
}

Ui::SetupDialog *SetupDialog::get_ui()
{
    return _ui;
}

QString SetupDialog::pageTitle() const
{
    return _ui->titleLabel->text();
}
#endif

void SetupDialog::skipWizard()
{
    // Remove any saved settings
    TSettings settings;
    settings.clear();

    // Indicate that we've finished (successfully)
    finishedWizard();
}

void SetupDialog::backButtonClicked()
{
    int nextIndex = _ui->wizardStackedWidget->currentIndex() - 1;
    if(nextIndex < 0)
        skipWizard();
    else
        _ui->wizardStackedWidget->setCurrentIndex(nextIndex);
}

void SetupDialog::nextButtonClicked()
{
    if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
        registerMachine();
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->donePage)
        finishedWizard();
    else
        setNextPage();
}

void SetupDialog::setNextPage()
{
    if(_ui->wizardStackedWidget->currentWidget() == _ui->welcomePage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->cliPage);
        // Disable this until we know the version number.
        _ui->nextButton->setEnabled(false);
        bool advancedOk = validateCLIPage();
        _ui->cliAdvancedButton->setChecked(!advancedOk);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->cliPage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->registerPage);
        if(validateRegisterPage())
            _ui->nextButton->setFocus();
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->donePage);
    }
}

void SetupDialog::showTarsnapPathBrowse()
{
    QString tarsnapPath =
        QFileDialog::getExistingDirectory(this, tr("Find Tarsnap client"),
                                          _ui->cliPathLineBrowse->text());
    _ui->cliPathLineBrowse->setText(tarsnapPath);
}

void SetupDialog::showTarsnapCacheBrowse()
{
    QString tarsnapCacheDir =
        QFileDialog::getExistingDirectory(this, tr("Tarsnap cache location"),
                                          _ui->cachePathLineBrowse->text());
    _ui->cachePathLineBrowse->setText(tarsnapCacheDir);
}

void SetupDialog::showAppDataBrowse()
{
    QString appDataDir =
        QFileDialog::getExistingDirectory(this, tr("App data location"),
                                          _ui->appdataPathLineBrowse->text());
    _ui->appdataPathLineBrowse->setText(appDataDir);
}

bool SetupDialog::validateCLIPage()
{
    QString tarsnapDir;
    QString appDataDir;
    QString tarsnapCacheDir;
    QString errorMsg;

    bool result = true;

    appDataDir = _ui->appdataPathLineBrowse->text();
    errorMsg   = Utils::validate_writeable_dir(appDataDir);
    if(!errorMsg.isEmpty())
    {
        _ui->cliValidationLabel->setText(tr("Invalid App data directory set."));
        result = false;
    }
    else
    {
        TSettings settings;
        appDataDir = QFileInfo(appDataDir).canonicalFilePath();
        settings.setValue("app/app_data", appDataDir);
    }

    tarsnapCacheDir = _ui->cachePathLineBrowse->text();
    errorMsg        = Utils::validate_writeable_dir(tarsnapCacheDir);
    if(result && !errorMsg.isEmpty())
    {
        _ui->cliValidationLabel->setText(tr("Invalid Tarsnap cache directory"
                                            " set."));
        result = false;
    }

    struct DirMessage cliDirMessage =
        Utils::findTarsnapClientInPath(_ui->cliPathLineBrowse->text(), true);
    tarsnapDir = cliDirMessage.dirname;
    if(result && tarsnapDir.isEmpty())
    {
        _ui->cliValidationLabel->setText(
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

void SetupDialog::createKeyfile()
{
    _ui->registerKeyStackedWidget->setCurrentWidget(_ui->createKeyfileSubpage);
    // Share machineNameLineEdit in both pages of the keyStackedWidget
    _ui->gridKeyNoLayout->addWidget(_ui->machineNameLineEdit, 1, 1);
    _ui->statusLabel->clear();
    if(validateRegisterPage())
        _ui->nextButton->setFocus();
}

void SetupDialog::useExistingKeyfile()
{
    _ui->registerKeyStackedWidget->setCurrentWidget(
        _ui->useExistingKeyfileSubpage);
    // Share machineNameLineEdit in both pages of the keyStackedWidget
    _ui->gridKeyYesLayout->addWidget(_ui->machineNameLineEdit, 1, 1);
    _ui->statusLabel->clear();
    if(validateRegisterPage())
        _ui->nextButton->setFocus();
}

bool SetupDialog::validateRegisterPage()
{
    bool result = false;
    if(_ui->useExistingKeyfileButton->isChecked())
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

    QString tarsnapKeyFile;
    QString appDataDir;

    // Sanity check app data dir.
    appDataDir = settings.value("app/app_data", "").toString();
    if(appDataDir.isEmpty())
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
    if(_ui->useExistingKeyfileButton->isChecked())
    {
        useExistingKeyfile = true;
        _ui->statusLabel->setText("Verifying archive integrity...");
        tarsnapKeyFile = _ui->machineKeyCombo->currentText();
    }
    else
    {
        _ui->statusLabel->setText("Generating keyfile...");
        tarsnapKeyFile =
            appDataDir + QDir::separator() + _ui->machineNameLineEdit->text()
            + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")
            + ".key";
    }

    settings.setValue("tarsnap/cache", _ui->cachePathLineBrowse->text());
    settings.setValue("tarsnap/key", tarsnapKeyFile);
    settings.setValue("tarsnap/user", _ui->tarsnapUserLineEdit->text());
    settings.setValue("tarsnap/machine", _ui->machineNameLineEdit->text());

    emit registerMachineRequested(_ui->tarsnapPasswordLineEdit->text(),
                                  useExistingKeyfile);
}

void SetupDialog::registerMachineResponse(TaskStatus status, QString reason)
{
    TSettings settings;

    // Get keyfile and sanity check.
    QString tarsnapKeyFile = settings.value("tarsnap/key", "").toString();
    if((status == TaskStatus::Completed) && (tarsnapKeyFile.isEmpty()))
    {
        // This should never happen
        status = TaskStatus::Failed;
        reason = "No keyfile set";
    }

    switch(status)
    {
    case TaskStatus::Completed:
        _ui->statusLabel->clear();

        _ui->doneKeyFileNameLabel->setText(
            QString("<a href=\"%1\">%2</a>")
                .arg(QUrl::fromLocalFile(
                         QFileInfo(tarsnapKeyFile).absolutePath())
                         .toString())
                .arg(tarsnapKeyFile));
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
        _ui->cliValidationLabel->setText(tr("Tarsnap CLI version ")
                                         + versionString + tr(" detected.  ✔"));
        // Record value
        settings.setValue("tarsnap/version", versionString);
        // Enable progress
        _ui->nextButton->setEnabled(true);
        _ui->nextButton->setFocus();
        break;
    case TaskStatus::VersionTooLow:
        // Don't record the too-low version number.
        _ui->cliValidationLabel->setText(
            tr("Tarsnap CLI version ") + versionString
            + tr(" too low; must be at least %1").arg(TARSNAP_MIN_VERSION));
        _ui->cliAdvancedButton->setChecked(true);
        break;
    case TaskStatus::Failed:
        _ui->cliValidationLabel->setText(
            tr("Error retrieving Tarsnap CLI verison"));
        _ui->cliAdvancedButton->setChecked(true);
        break;
    default:
        break;
    }
}

void SetupDialog::finishedWizard()
{
    TSettings settings;

    // We've either completed the setup wizard, or deliberately skipped it.
    settings.setValue("app/wizard_done", true);

    accept();
}

void SetupDialog::reject()
{
    // Remove any saved settings
    TSettings settings;
    settings.clear();

    // Call parent function
    QDialog::reject();
}

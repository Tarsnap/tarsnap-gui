#include "setupdialog.h"
#include "debug.h"
#include "ui_setupdialog.h"
#include "utils.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHostInfo>
#include <QSettings>
#include <QStandardPaths>

SetupDialog::SetupDialog(QWidget *parent)
    : QDialog(parent),
      _ui(new Ui::SetupDialog),
      _loadingAnimation(":/icons/loading.gif"),
      _haveKey(false)
{
    _ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) &
                   ~Qt::WindowMaximizeButtonHint);

    _ui->loadingIconLabel->setMovie(&_loadingAnimation);
    _ui->clientVersionLabel->hide();
    _ui->errorLabel->hide();
    _ui->machineKeyLabel->hide();
    _ui->machineKeyCombo->hide();
    _ui->browseKeyButton->hide();

    connect(_ui->welcomePageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);
    connect(_ui->restorePageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);
    connect(_ui->advancedPageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);
    connect(_ui->registerPageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);
    connect(_ui->donePageRadioButton, &QRadioButton::clicked, this,
            &SetupDialog::skipToPage);

    connect(_ui->wizardStackedWidget, &QStackedWidget::currentChanged, this,
            &SetupDialog::wizardPageChanged);

    // Welcome page
    connect(_ui->welcomePageSkipButton, &QPushButton::clicked,
            [&]() { commitSettings(true); });
    connect(_ui->welcomePageProceedButton, &QPushButton::clicked, this,
            &SetupDialog::setNextPage);

    // Advanced setup page
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
    connect(_ui->advancedPageProceedButton, &QPushButton::clicked, this,
            &SetupDialog::setNextPage);

    // Restore page
    connect(_ui->restoreNoButton, &QPushButton::clicked, this,
            &SetupDialog::restoreNo);
    connect(_ui->restoreYesButton, &QPushButton::clicked, this,
            &SetupDialog::restoreYes);

    // Register page
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
    connect(_ui->registerMachineButton, &QPushButton::clicked, this,
            &SetupDialog::registerMachine);

    // Done page
    connect(_ui->doneButton, &QPushButton::clicked, this,
            &SetupDialog::commitSettings);

    _appDataDir = QStandardPaths::writableLocation(APPDATA);
    QDir keysDir(_appDataDir);
    if(!keysDir.exists())
        keysDir.mkpath(_appDataDir);
    _ui->appDataPathLineEdit->setText(_appDataDir);

    _tarsnapCacheDir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(_tarsnapCacheDir);
    if(!cacheDir.exists())
        cacheDir.mkpath(_tarsnapCacheDir);
    _ui->tarsnapCacheLineEdit->setText(_tarsnapCacheDir);

    _tarsnapDir = Utils::findTarsnapClientInPath("", true);
    _ui->tarsnapPathLineEdit->setText(_tarsnapDir);
    _ui->machineNameLineEdit->setText(QHostInfo::localHostName());
    _ui->wizardStackedWidget->setCurrentWidget(_ui->welcomePage);
}

SetupDialog::~SetupDialog()
{
    delete _ui;
}

void SetupDialog::wizardPageChanged(int)
{
    if(_ui->wizardStackedWidget->currentWidget() == _ui->welcomePage)
    {
        _ui->welcomePageRadioButton->setChecked(true);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->advancedPage)
    {
        _ui->advancedPageRadioButton->setChecked(true);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->restorePage)
    {
        _ui->restorePageRadioButton->setChecked(true);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
    {
        _ui->registerPageRadioButton->setChecked(true);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->donePage)
    {
        _ui->donePageRadioButton->setChecked(true);
    }
}

void SetupDialog::skipToPage()
{
    if(sender() == _ui->welcomePageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->welcomePage);
    else if(sender() == _ui->restorePageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->restorePage);
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
        validateAdvancedSetupPage();
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->advancedPage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->restorePage);
        _ui->restorePageRadioButton->setEnabled(true);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->restorePage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->registerPage);
        _ui->registerPageRadioButton->setEnabled(true);
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
    bool result = false;

    setTarsnapVersion("");

    _tarsnapDir =
        Utils::findTarsnapClientInPath(_ui->tarsnapPathLineEdit->text(), true);
    _tarsnapCacheDir =
        Utils::validateTarsnapCache(_ui->tarsnapCacheLineEdit->text());
    QFileInfo appDataDir(_ui->appDataPathLineEdit->text());
    if(appDataDir.exists() && appDataDir.isDir() && appDataDir.isWritable())
        _appDataDir = _ui->appDataPathLineEdit->text();
    else
        _appDataDir.clear();

    if(_tarsnapDir.isEmpty() || _tarsnapCacheDir.isEmpty() ||
       _appDataDir.isEmpty())
        result = false;
    else
        result = true;

    if(result)
        emit getTarsnapVersion(_tarsnapDir);

    _ui->advancedPageProceedButton->setEnabled(result);

    return result;
}

void SetupDialog::restoreNo()
{
    _haveKey = false;
    _ui->machineKeyLabel->hide();
    _ui->machineKeyCombo->hide();
    _ui->browseKeyButton->hide();
    _ui->tarsnapUserLabel->show();
    _ui->tarsnapUserLineEdit->show();
    _ui->tarsnapPasswordLabel->show();
    _ui->tarsnapPasswordLineEdit->show();
    _ui->registerPageInfoLabel->setText(
        tr("Please use your Tarsnap account "
           "credentials. Don't have an account? "
           "Register one <a href=\"http://tarsnap.com\">here</a>."));
    _ui->errorLabel->clear();
    setNextPage();
}

void SetupDialog::restoreYes()
{
    _haveKey = true;
    _ui->tarsnapUserLabel->hide();
    _ui->tarsnapUserLineEdit->hide();
    _ui->tarsnapPasswordLabel->hide();
    _ui->tarsnapPasswordLineEdit->hide();
    _ui->machineKeyLabel->show();
    _ui->machineKeyCombo->show();
    _ui->browseKeyButton->show();
    _ui->registerPageInfoLabel->setText(
        tr("Please use your existing machine key "
           "and a machine name of your liking. "
           "The registration might take a bit to "
           "verify archive consistency and integrity "
           "using the cache, thus please be patient."));
    _ui->errorLabel->clear();
    _ui->machineKeyCombo->clear();
    foreach(QFileInfo file, Utils::findKeysInPath(_appDataDir))
        _ui->machineKeyCombo->addItem(file.canonicalFilePath());
    setNextPage();
}

void SetupDialog::validateRegisterPage()
{
    bool result = false;
    if(_haveKey)
    {
        // user specified key
        QFileInfo machineKeyFile(_ui->machineKeyCombo->currentText());
        if(!_ui->machineNameLineEdit->text().isEmpty() &&
           machineKeyFile.exists() && machineKeyFile.isFile() &&
           machineKeyFile.isReadable())
        {
            result = true;
        }
    }
    else
    {
        if(!_ui->tarsnapUserLineEdit->text().isEmpty() &&
           !_ui->tarsnapPasswordLineEdit->text().isEmpty() &&
           !_ui->machineNameLineEdit->text().isEmpty())
        {
            result = true;
        }
    }

    _ui->registerMachineButton->setEnabled(result);
}

void SetupDialog::registerHaveKeyBrowse()
{
    QString existingMachineKey =
        QFileDialog::getOpenFileName(this,
                                     tr("Browse for existing machine key"));
    if(!existingMachineKey.isEmpty())
        _ui->machineKeyCombo->setCurrentText(existingMachineKey);
}

void SetupDialog::registerMachine()
{
    _ui->registerMachineButton->setEnabled(false);
    _ui->errorLabel->clear();
    if(_haveKey)
        _tarsnapKeyFile = _ui->machineKeyCombo->currentText();
    else
        _tarsnapKeyFile =
            _appDataDir + QDir::separator() + _ui->machineNameLineEdit->text() +
            "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss") +
            ".key";

    DEBUG << "Registration details >>\n"
          << _tarsnapDir << ::endl
          << _appDataDir << ::endl
          << _tarsnapKeyFile << ::endl
          << _tarsnapCacheDir;

    emit requestRegisterMachine(_ui->tarsnapUserLineEdit->text(),
                                _ui->tarsnapPasswordLineEdit->text(),
                                _ui->machineNameLineEdit->text(),
                                _tarsnapKeyFile, _tarsnapDir, _tarsnapCacheDir);
}

void SetupDialog::registerMachineStatus(TaskStatus status, QString reason)
{
    switch(status)
    {
    case TaskStatus::Completed:
        _ui->errorLabel->clear();
        _ui->doneInfoTextBrowser->setHtml(_ui->doneInfoTextBrowser->toHtml()
                                              .arg(_tarsnapKeyFile)
                                              .arg(_tarsnapKeyFile));
        _ui->doneButton->setEnabled(true);
        setNextPage();
        break;
    case TaskStatus::Failed:
        _ui->errorLabel->setText(reason);
        _ui->errorLabel->show();
        _ui->registerMachineButton->setEnabled(true);
        resize(sizeHint());
        break;
    default:
        break;
    }
}

void SetupDialog::updateLoadingAnimation(bool idle)
{
    if(idle)
    {
        _loadingAnimation.stop();
        _ui->loadingIconLabel->hide();
    }
    else
    {
        _loadingAnimation.start();
        _ui->loadingIconLabel->show();
    }
}

void SetupDialog::setTarsnapVersion(QString versionString)
{
    _tarsnapVersion = versionString;
    if(_tarsnapVersion.isEmpty())
    {
        _ui->clientVersionLabel->clear();
        _ui->clientVersionLabel->hide();
    }
    else
    {
        _ui->clientVersionLabel->setText(tr("Tarsnap version ") +
                                         _tarsnapVersion + tr(" detected"));
        _ui->clientVersionLabel->show();
    }
}

void SetupDialog::commitSettings(bool skipped)
{
    QSettings settings;

    DEBUG << "Settings location is " << settings.fileName();

    settings.setValue("app/wizard_done", true);

    if(!skipped)
    {
        QSettings settings;
        settings.setValue("app/app_data", _appDataDir);
        settings.setValue("tarsnap/path", _tarsnapDir);
        settings.setValue("tarsnap/version", _tarsnapVersion);
        settings.setValue("tarsnap/cache", _tarsnapCacheDir);
        settings.setValue("tarsnap/key", _tarsnapKeyFile);
        settings.setValue("tarsnap/user", _ui->tarsnapUserLineEdit->text());
        settings.setValue("tarsnap/machine", _ui->machineNameLineEdit->text());
    }
    settings.sync();

    accept();
}

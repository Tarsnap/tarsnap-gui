#include "setupdialog.h"
#include "ui_setupdialog.h"
#include "utils.h"
#include "debug.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QHostInfo>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QSettings>

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SetupDialog),
    _loadingAnimation(":/icons/loading.gif"),
    _haveKey(false)
{
    _ui->setupUi(this);

    _ui->loadingIconLabel->setMovie(&_loadingAnimation);
    _ui->advancedPageRadioButton->hide();
    _ui->errorLabel->hide();
    _ui->machineKeyLabel->hide();
    _ui->machineKeyLineEdit->hide();
    _ui->locateMachineKeyLabel->hide();

    connect(_ui->welcomePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->restorePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->advancedPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->registerPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->donePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));

    connect(_ui->wizardStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(wizardPageChanged(int)));

    // Welcome page
    connect(_ui->advancedSetupCheckBox, SIGNAL(toggled(bool)), _ui->advancedPageRadioButton, SLOT(setVisible(bool)));
    connect(_ui->welcomePageSkipButton, &QPushButton::clicked, [=](){commitSettings(true);});
    connect(_ui->welcomePageProceedButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    // Advanced setup page
    connect(_ui->tarsnapPathBrowseButton, SIGNAL(clicked()), this, SLOT(showTarsnapPathBrowse()));
    connect(_ui->tarsnapPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateAdvancedSetupPage()));
    connect(_ui->tarsnapCacheBrowseButton, SIGNAL(clicked()), this, SLOT(showTarsnapCacheBrowse()));
    connect(_ui->tarsnapCacheLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateAdvancedSetupPage()));
    connect(_ui->appDataBrowseButton, SIGNAL(clicked()), this, SLOT(showAppDataBrowse()));
    connect(_ui->appDataPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateAdvancedSetupPage()));
    connect(_ui->advancedPageProceedButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    // Restore page
    connect(_ui->restoreNoButton, SIGNAL(clicked()), this, SLOT(restoreNo()));
    connect(_ui->restoreYesButton, SIGNAL(clicked()), this, SLOT(restoreYes()));

    // Register page
    connect(_ui->tarsnapUserLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(_ui->tarsnapPasswordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(_ui->machineNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(_ui->machineKeyLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(_ui->locateMachineKeyLabel, SIGNAL(linkActivated(QString)), this, SLOT(registerHaveKeyBrowse(QString)));
    connect(_ui->registerMachineButton, SIGNAL(clicked()), this, SLOT(registerMachine()));

    // Done page
    connect(_ui->doneButton, SIGNAL(clicked()), this, SLOT(commitSettings()));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    _appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#else
    _appDataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    QDir keysDir(_appDataDir);
    if(!keysDir.exists())
        keysDir.mkpath(_appDataDir);
    _ui->appDataPathLineEdit->setText(_appDataDir);

    _tarsnapCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(_tarsnapCacheDir);
    if(!cacheDir.exists())
        cacheDir.mkpath(_tarsnapCacheDir);
    _ui->tarsnapCacheLineEdit->setText(_tarsnapCacheDir);

    _tarsnapCLIDir = Utils::findTarsnapClientInPath("", true);
    _ui->tarsnapPathLineEdit->setText(_tarsnapCLIDir);
    _ui->machineNameLineEdit->setText(QHostInfo::localHostName());
    _ui->wizardStackedWidget->setCurrentWidget(_ui->welcomePage);
}

SetupDialog::~SetupDialog()
{
    delete _ui;
}

void SetupDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        _windowDragPos = event->pos();
}

void SetupDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        QPoint diff = event->pos() - _windowDragPos;
        QPoint newpos = this->pos() + diff;
        this->move(newpos);
    }
}

void SetupDialog::wizardPageChanged(int)
{
    if(_ui->wizardStackedWidget->currentWidget() == _ui->welcomePage)
    {
        _ui->welcomePageRadioButton->setChecked(true);
    }
    else
    if(_ui->wizardStackedWidget->currentWidget() == _ui->advancedPage)
    {
        _ui->advancedPageRadioButton->setChecked(true);
    }
    else
    if(_ui->wizardStackedWidget->currentWidget() == _ui->restorePage)
    {
        _ui->restorePageRadioButton->setChecked(true);
    }
    else
    if(_ui->wizardStackedWidget->currentWidget() == _ui->registerPage)
    {
        _ui->registerPageRadioButton->setChecked(true);
    }
    else
    if(_ui->wizardStackedWidget->currentWidget() == _ui->donePage)
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
        if(_ui->advancedSetupCheckBox->isChecked() || _tarsnapCLIDir.isEmpty()
           || _tarsnapCacheDir.isEmpty() || _appDataDir.isEmpty())
        {
            _ui->wizardStackedWidget->setCurrentWidget(_ui->advancedPage);
            _ui->advancedSetupCheckBox->setChecked(true);
            _ui->advancedPageRadioButton->setEnabled(true);
        }
        else
        {
            _ui->wizardStackedWidget->setCurrentWidget(_ui->restorePage);
            _ui->restorePageRadioButton->setEnabled(true);
        }
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
    QString tarsnapPath = QFileDialog::getExistingDirectory(this,
                                                            tr("Find Tarsnap client"), "");
    _ui->tarsnapPathLineEdit->setText(tarsnapPath);
}

void SetupDialog::showTarsnapCacheBrowse()
{
    QString tarsnapCacheDir = QFileDialog::getExistingDirectory(this,
                                                            tr("Tarsnap cache location"), _tarsnapCacheDir);
    _ui->tarsnapCacheLineEdit->setText(tarsnapCacheDir);
}

void SetupDialog::showAppDataBrowse()
{
    QString appDataDir = QFileDialog::getExistingDirectory(this,
                                                            tr("App data location"), "");
    _ui->appDataPathLineEdit->setText(appDataDir);
}

bool SetupDialog::validateAdvancedSetupPage()
{
    bool result = false;

    _tarsnapCLIDir   = Utils::findTarsnapClientInPath(_ui->tarsnapPathLineEdit->text(), true);
    _tarsnapCacheDir = Utils::validateTarsnapCache(_ui->tarsnapCacheLineEdit->text());
    QFileInfo appDataDir(_ui->appDataPathLineEdit->text());
    if(appDataDir.exists() && appDataDir.isDir() && appDataDir.isWritable())
        _appDataDir = _ui->appDataPathLineEdit->text();
    else
        _appDataDir.clear();

    if(_tarsnapCLIDir.isEmpty() || _tarsnapCacheDir.isEmpty() || _appDataDir.isEmpty())
        result = false;
    else
        result = true;

    _ui->advancedPageProceedButton->setEnabled(result);

    return result;
}

void SetupDialog::restoreNo()
{
    _haveKey = false;
    _ui->machineKeyLabel->hide();
    _ui->machineKeyLineEdit->hide();
    _ui->locateMachineKeyLabel->hide();
    _ui->tarsnapUserLabel->show();
    _ui->tarsnapUserLineEdit->show();
    _ui->tarsnapPasswordLabel->show();
    _ui->tarsnapPasswordLineEdit->show();
    _ui->registerPageInfoLabel->setText(tr("Please use your Tarsnap account "
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
    _ui->machineKeyLineEdit->show();
    _ui->locateMachineKeyLabel->show();
    _ui->registerPageInfoLabel->setText(tr("Please use your existing machine key "
                                           "and a machine name of your liking. "
                                           "The registration might take a bit to "
                                           "verify archive consistency and integrity "
                                           "using the cache, thus please be patient."));
    _ui->errorLabel->clear();
    setNextPage();
}

void SetupDialog::validateRegisterPage()
{
    bool result = false;
    if(_haveKey)
    {
        // user specified key
        QFileInfo machineKeyFile(_ui->machineKeyLineEdit->text());
        if(!_ui->machineNameLineEdit->text().isEmpty()
           && machineKeyFile.exists() && machineKeyFile.isFile() && machineKeyFile.isReadable())
            result = true;
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

    _ui->registerMachineButton->setEnabled(result);
}

void SetupDialog::registerHaveKeyBrowse(QString url)
{
    Q_UNUSED(url);
    QString existingMachineKey = QFileDialog::getOpenFileName(this
                                                              ,tr("Browse for existing machine key"));
    _ui->machineKeyLineEdit->setText(existingMachineKey);
}

void SetupDialog::registerMachine()
{
    _ui->registerMachineButton->setEnabled(false);
    _ui->errorLabel->clear();
    if(_haveKey)
    {
        _tarsnapKeyFile = _ui->machineKeyLineEdit->text();
    }
    else
    {
        _tarsnapKeyFile = _appDataDir + QDir::separator() + _ui->machineNameLineEdit->text()
                          + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")
                          + ".key";
    }
    DEBUG << "Registration details >>\n" << _tarsnapCLIDir << ::endl << _appDataDir << ::endl
          << _tarsnapKeyFile << ::endl << _tarsnapCacheDir;

    emit registerMachine(_ui->tarsnapUserLineEdit->text(), _ui->tarsnapPasswordLineEdit->text()
                         , _ui->machineNameLineEdit->text(), _tarsnapKeyFile, _tarsnapCLIDir
                         , _tarsnapCacheDir);
}

void SetupDialog::registerMachineStatus(TaskStatus status, QString reason)
{
    switch(status)
    {
        case TaskStatus::Completed:
            _ui->errorLabel->clear();
            _ui->doneInfoTextBrowser->setHtml(_ui->doneInfoTextBrowser->toHtml().arg(_tarsnapKeyFile).arg(_tarsnapKeyFile));
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

void SetupDialog::commitSettings(bool skipped)
{
    QSettings settings;

    DEBUG << "Settings location is " << settings.fileName();

    settings.setValue("app/wizard_done", true);

    if(!skipped)
    {
        if(_ui->storeIniFormatCheckBox->isChecked())
        {
            settings.setValue("app/ini_format", true);
            settings.setValue("app/app_data", _appDataDir);
            settings.sync();
            settings.setPath(QSettings::IniFormat, QSettings::UserScope, _appDataDir);
            settings.setDefaultFormat(QSettings::IniFormat);
        }

        QSettings settings;
        settings.setValue("app/app_data",    _appDataDir);
        settings.setValue("tarsnap/path",   _tarsnapCLIDir);
        settings.setValue("tarsnap/cache",  _tarsnapCacheDir);
        settings.setValue("tarsnap/key",    _tarsnapKeyFile);
        settings.setValue("tarsnap/user",   _ui->tarsnapUserLineEdit->text());
        settings.setValue("tarsnap/machine", _ui->machineNameLineEdit->text());
    }
    settings.sync();

    accept();
}

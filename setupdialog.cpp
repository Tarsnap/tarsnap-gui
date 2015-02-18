#include "setupdialog.h"
#include "ui_setupdialog.h"
#include "utils.h"

#include <QDebug>
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
    _loadingAnimation(":/resources/loading.gif"),
    _haveKey(false)
{
    _ui->setupUi(this);

    _ui->loadingIconLabel->setMovie(&_loadingAnimation);
    showAdvancedSetup(false);
    _ui->errorLabel->hide();
    _ui->machineKeyLabel->hide();
    _ui->machineKeyLineEdit->hide();
    _ui->locateMachineKeyLabel->hide();

    connect(_ui->welcomePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->restorePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->advancedPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->registerAccountPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(_ui->donePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));

    connect(_ui->wizardStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(wizardPageChanged(int)));

    // Welcome page
    connect(_ui->advancedSetupCheckBox, SIGNAL(toggled(bool)), this, SLOT(showAdvancedSetup(bool)));
    connect(_ui->welcomePageCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_ui->welcomePageProceedButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    // Advanced setup page
    connect(_ui->tarsnapPathBrowseButton, SIGNAL(clicked()), this, SLOT(showTarsnapPathBrowse()));
    connect(_ui->tarsnapPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateAdvancedSetupPage()));
    connect(_ui->tarsnapCacheBrowseButton, SIGNAL(clicked()), this, SLOT(showTarsnapCacheBrowse()));
    connect(_ui->tarsnapCacheLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateAdvancedSetupPage()));
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
    _tarsnapKeysDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#else
    _tarsnapKeysDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
    QDir keysDir(_tarsnapKeysDir);
    if(!keysDir.exists())
        keysDir.mkpath(_tarsnapKeysDir);

    _tarsnapCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(_tarsnapCacheDir);
    if(!cacheDir.exists())
        cacheDir.mkpath(_tarsnapCacheDir);
    _ui->tarsnapCacheLineEdit->setText(_tarsnapCacheDir);

    qDebug() << _tarsnapKeysDir << ::endl << _tarsnapCacheDir;

    findTarsnapInPath();

    _ui->machineNameLineEdit->setText(QHostInfo::localHostName());

    _ui->wizardStackedWidget->setCurrentWidget(_ui->welcomePage);
}

SetupDialog::~SetupDialog()
{
    delete _ui;
}

void SetupDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _windowDragPos = event->pos();
    }
}

void SetupDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
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
    if(_ui->wizardStackedWidget->currentWidget() == _ui->registerAccountPage)
    {
        _ui->registerAccountPageRadioButton->setChecked(true);
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
    else if(sender() == _ui->registerAccountPageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->registerAccountPage);
    else if(sender() == _ui->donePageRadioButton)
        _ui->wizardStackedWidget->setCurrentWidget(_ui->donePage);
}

void SetupDialog::showAdvancedSetup(bool display)
{
    _ui->advancedPageRadioButton->setVisible(display);
}

void SetupDialog::setNextPage()
{
    if(_ui->wizardStackedWidget->currentWidget() == _ui->welcomePage)
    {
        if(_ui->advancedSetupCheckBox->isChecked() || _tarsnapCLIDir.isEmpty()
           || _tarsnapCacheDir.isEmpty() || _tarsnapKeysDir.isEmpty())
        {
            showAdvancedSetup(true);
            _ui->wizardStackedWidget->setCurrentWidget(_ui->advancedPage);
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
        _ui->wizardStackedWidget->setCurrentWidget(_ui->registerAccountPage);
        _ui->registerAccountPageRadioButton->setEnabled(true);
    }
    else if(_ui->wizardStackedWidget->currentWidget() == _ui->registerAccountPage)
    {
        _ui->wizardStackedWidget->setCurrentWidget(_ui->donePage);
        _ui->donePageRadioButton->setEnabled(true);
    }
}

void SetupDialog::showTarsnapPathBrowse()
{
    QString tarsnapPath = QFileDialog::getExistingDirectory(this,
                                                            tr("Find Tarsnap client")
                                                            , "");
    _ui->tarsnapPathLineEdit->setText(tarsnapPath);
}

void SetupDialog::showTarsnapCacheBrowse()
{
    QString tarsnapCacheDir = QFileDialog::getExistingDirectory(this,
                                                            tr("Tarsnap cache location"),
                                                            _tarsnapCacheDir);
    _ui->tarsnapCacheLineEdit->setText(tarsnapCacheDir);
}

bool SetupDialog::validateAdvancedSetupPage()
{
    bool result = false;

    QString tarsnapPath = Utils::validateTarsnapPath(_ui->tarsnapPathLineEdit->text());
    QString tarsnapCache = Utils::validateTarsnapCache(_ui->tarsnapCacheLineEdit->text());

    if(!tarsnapPath.isEmpty())
    {
        _tarsnapCLIDir = tarsnapPath;
        result = true;
    }

    if(!tarsnapCache.isEmpty())
    {
        _tarsnapCacheDir = tarsnapCache;
        result = result && true;
    }
    else
    {
        result = false;
    }

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
        _tarsnapKeyFile = _tarsnapKeysDir + QDir::separator() + _ui->machineNameLineEdit->text()
                          + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    }

    qDebug() << "Registration details >>\n " << _tarsnapCLIDir << ::endl
             << _tarsnapKeysDir << ::endl << _tarsnapKeyFile << ::endl
             << _tarsnapCacheDir;

    emit registerMachine(_ui->tarsnapUserLineEdit->text(), _ui->tarsnapPasswordLineEdit->text()
                         , _ui->machineNameLineEdit->text(), _tarsnapKeyFile, _tarsnapCLIDir, _tarsnapCacheDir);
}

void SetupDialog::registerMachineStatus(JobStatus status, QString reason)
{
    switch(status)
    {
        case JobStatus::Completed:
            _ui->errorLabel->clear();
            _ui->doneInfoTextBrowser->setHtml(_ui->doneInfoTextBrowser->toHtml().arg(_tarsnapKeyFile).arg(_tarsnapKeyFile));
            _ui->doneButton->setEnabled(true);
            setNextPage();
            break;
        case JobStatus::Failed:
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

void SetupDialog::findTarsnapInPath()
{
    // Maybe use QStandardPaths::​findExecutable instead of manual PATH search
    QStringList path = QString::fromUtf8(::getenv("PATH")).split(':', QString::SkipEmptyParts);
    qDebug() << "Will look for tarsnap in PATH: " << path;
    auto dir = path.begin();
    while (dir != path.end())
    {
        QString path = Utils::validateTarsnapPath(*dir);
        if(!path.isEmpty())
        {
            _tarsnapCLIDir = path;
            _ui->tarsnapPathLineEdit->setText(_tarsnapCLIDir);
            return;
        }
        ++dir;
    }
}

void SetupDialog::commitSettings()
{
    QSettings settings;

    settings.setValue("app/appdata",    _tarsnapKeysDir);
    settings.setValue("tarsnap/path",   _tarsnapCLIDir);
    settings.setValue("tarsnap/cache",  _tarsnapCacheDir);
    settings.setValue("tarsnap/key",    _tarsnapKeyFile);
    settings.setValue("tarsnap/user",   _ui->tarsnapUserLineEdit->text());
    settings.setValue("tarsnap/machine", _ui->machineNameLineEdit->text());
    settings.sync();
    qDebug() << settings.fileName();
    accept();
}

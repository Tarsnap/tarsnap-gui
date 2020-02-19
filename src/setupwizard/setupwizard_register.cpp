#include "setupwizard_register.h"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QFileDialog>

// This is used for QHostInfo::localHostName().  It could be replaced with
// QSysInfo::machineHostName() to remove the dependency on the Qt network
// library, but only if we require Qt 5.6+.
#include <QHostInfo>

#include "ui_setupwizard_register.h"
WARNINGS_ENABLE

#include <TSettings.h>
#include <TWizardPage.h>

#include "taskstatus.h"
#include "utils.h"

RegisterPage::RegisterPage(QWidget *parent)
    : TWizardPage(parent),
      _ui(new Ui::RegisterPage),
      _createKey(true),
      _registering(No)
{
    _ui->setupUi(this);

    // Basic operations on this page.
    connect(_ui->createKeyfileButton, &QPushButton::clicked, this,
            &RegisterPage::createKeyfile);
    connect(_ui->useExistingKeyfileButton, &QPushButton::clicked, this,
            &RegisterPage::useExistingKeyfile);

    // A config field changed.
    connect(_ui->machineNameLineEdit, &QLineEdit::textChanged, this,
            &RegisterPage::checkComplete);
    connect(_ui->tarsnapUserLineEdit, &QLineEdit::textChanged, this,
            &RegisterPage::checkComplete);
    connect(_ui->tarsnapPasswordLineEdit, &QLineEdit::textChanged, this,
            &RegisterPage::checkComplete);
    connect(_ui->keyfilePathComboBrowse, &PathComboBrowse::textChanged, this,
            &RegisterPage::checkComplete);
}

RegisterPage::~RegisterPage()
{
    delete _ui;
}

void RegisterPage::initializePage()
{
    TWizardPage::initializePage();

    // Default machine name.
    _ui->machineNameLineEdit->setText(QHostInfo::localHostName());

    // Find any existing keys.
    TSettings settings;
    QString   appDataDir = settings.value("app/app_data", "").toString();
    for(const QFileInfo &file : Utils::findKeysInPath(appDataDir))
        _ui->keyfilePathComboBrowse->addItem(file.canonicalFilePath());

    // Auto-select "use existing" if we have any.
    if(_ui->keyfilePathComboBrowse->count() > 0)
        _ui->useExistingKeyfileButton->click();

    // Enable keyboard focus if we're ready to go.
    if(checkComplete())
        _ui->nextButton->setFocus();
}

void RegisterPage::createKeyfile()
{
    _createKey = true;
    _ui->registerKeyStackedWidget->setCurrentWidget(_ui->createKeyfileSubpage);

    _ui->statusLabel->clear();
    checkComplete();
}

void RegisterPage::useExistingKeyfile()
{
    _createKey = false;
    _ui->registerKeyStackedWidget->setCurrentWidget(
        _ui->useExistingKeyfileSubpage);

    _ui->statusLabel->clear();
    checkComplete();
}

void RegisterPage::reportError(const QString &text)
{
    _ui->statusLabel->messageError(text);
}

void RegisterPage::next()
{
    if(_registering == Done)
    {
        emit nextPage();
        return;
    }
    else
    {
        registerMachine();
    }
}

bool RegisterPage::checkComplete()
{
    // Check mandatory fields (depending on which tab we're on).
    if(_createKey)
    {
        if(_ui->machineNameLineEdit->text().isEmpty())
            return setProceedButton(false);

        if(_ui->tarsnapUserLineEdit->text().isEmpty())
            return setProceedButton(false);

        if(_ui->tarsnapPasswordLineEdit->text().isEmpty())
            return setProceedButton(false);
    }
    else
    {
        if(!checkKeyfile(_ui->keyfilePathComboBrowse->text()))
            return setProceedButton(false);
    }

    // Disable the button if we're already registering.
    if(_registering == Yes)
        return setProceedButton(false);
    else
        return setProceedButton(true);
}

void RegisterPage::registerMachine()
{
    TSettings settings;

    QString tarsnapKeyFile;
    QString appDataDir;

    // Sanity check app data dir.
    appDataDir = settings.value("app/app_data", "").toString();
    if(appDataDir.isEmpty())
    {
        // We should never get here, but handle the error anyway
        reportError("No app data dir set");
        checkComplete();
        return;
    }

    bool useExistingKeyfile = false;
    _ui->statusLabel->clear();
    if(_ui->useExistingKeyfileButton->isChecked())
    {
        useExistingKeyfile = true;
        _ui->statusLabel->messageNormal("Verifying archive integrity...");
        tarsnapKeyFile = _ui->keyfilePathComboBrowse->text();
    }
    else
    {
        _ui->statusLabel->messageNormal("Generating keyfile...");
        tarsnapKeyFile =
            appDataDir + QDir::separator() + _ui->machineNameLineEdit->text()
            + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")
            + ".key";
    }
    Q_ASSERT(!tarsnapKeyFile.isEmpty());

    settings.setValue("tarsnap/key", tarsnapKeyFile);
    settings.setValue("tarsnap/user", _ui->tarsnapUserLineEdit->text());

    if(checkComplete())
    {
        _registering = Yes;
        emit registerMachineRequested(_ui->tarsnapPasswordLineEdit->text(),
                                      _ui->machineNameLineEdit->text(),
                                      useExistingKeyfile);
    }
}

void RegisterPage::registerMachineResponse(TaskStatus status, QString reason)
{
    TSettings settings;

    // Get keyfile and sanity check.
    QString tarsnapKeyFile = settings.value("tarsnap/key", "").toString();
    if((status == TaskStatus::Completed) && (tarsnapKeyFile.isEmpty()))
    {
        // This should never happen.
        status = TaskStatus::Failed;
        reason = "No keyfile set";
    }

    switch(status)
    {
    case TaskStatus::Completed:
        _registering = Done;
        _ui->statusLabel->clear();
        emit next();
        break;
    case TaskStatus::Failed:
        reportError(reason);
        checkComplete();
        break;
    default:
        // We shouldn't receive anything else, so ignore it.
        break;
    }
}

bool RegisterPage::checkKeyfile(const QString &filename)
{
    if(filename.isEmpty())
        return false;

    QFileInfo machineKeyFile(filename);
    if(machineKeyFile.exists() && machineKeyFile.isFile()
       && machineKeyFile.isReadable())
    {
        _ui->statusLabel->clear();
        return true;
    }
    else
    {
        reportError("Invalid machine key");
        return false;
    }
}

void RegisterPage::updateLoadingAnimation(bool idle)
{
    _ui->busyWidget->animate(!idle);
}

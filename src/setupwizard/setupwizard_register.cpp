#include "setupwizard/setupwizard_register.h"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVariant>

// This is used for QHostInfo::localHostName().  It could be replaced with
// QSysInfo::machineHostName() to remove the dependency on the Qt network
// library, but only if we require Qt 5.6+.
#include <QHostInfo>

#include "ui_setupwizard_register.h"
WARNINGS_ENABLE

#include "TBusyLabel.h"
#include "TElidedLabel.h"
#include "TPathComboBrowse.h"
#include "TSettings.h"
#include "TWizardPage.h"

#include "messages/taskstatus.h"

#include "dir-utils.h"

RegisterPage::RegisterPage(QWidget *parent)
    : TWizardPage(parent), _ui(new Ui::RegisterPage), _registering(No)
{
    _ui->setupUi(this);

    // Basic operations on this page.
    connect(_ui->keyfileTabWidget, &QTabWidget::currentChanged, this,
            &RegisterPage::checkComplete);

    // A config field changed.
    connect(_ui->machineNameLineEdit, &QLineEdit::textChanged, this,
            &RegisterPage::checkComplete);
    connect(_ui->tarsnapUserLineEdit, &QLineEdit::textChanged, this,
            &RegisterPage::checkComplete);
    connect(_ui->tarsnapPasswordLineEdit, &QLineEdit::textChanged, this,
            &RegisterPage::checkComplete);
    connect(_ui->keyfilePathComboBrowse, &TPathComboBrowse::textChanged, this,
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
    for(const QFileInfo &file : findKeysInPath(appDataDir))
        _ui->keyfilePathComboBrowse->addItem(file.canonicalFilePath());

    // Auto-select "use existing" if we have any.
    if(_ui->keyfilePathComboBrowse->count() > 0)
        _ui->keyfileTabWidget->setCurrentIndex(UseKeyfileTab);

    // Enable keyboard focus if we're ready to go.
    if(checkComplete())
        _ui->nextButton->setFocus();
}

bool RegisterPage::reportError(const QString &text, TPathComboBrowse *pcb,
                               const QString &pcb_text)
{
    // General display.
    _ui->statusLabel->messageError(text);

    // Handle widget-specific display.
    if(pcb != nullptr)
        pcb->setStatusError(pcb_text);

    // Generic handling of errors.
    return false;
}

void RegisterPage::next()
{
    if(_registering == Done)
        emit nextPage();
    else
        registerMachine();
}

bool RegisterPage::checkComplete()
{
    _ui->statusLabel->clear();

    // Check mandatory fields (depending on which tab we're on).
    if(_ui->keyfileTabWidget->currentIndex() == CreateKeyfileTab)
    {
        if(!checkCreateKeyfile())
            return setProceedButton(false);
    }
    else
    {
        if(!checkUseKeyfile())
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

    QString tarsnapKeyFilename;
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

    // What type of operations are we doing?
    bool useExistingKeyfile =
        (_ui->keyfileTabWidget->currentIndex() == UseKeyfileTab);

    // Set the keyfilename.
    if(!useExistingKeyfile)
    {
        tarsnapKeyFilename =
            appDataDir + QDir::separator() + _ui->machineNameLineEdit->text()
            + "-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss")
            + ".key";
    }
    else
    {
        tarsnapKeyFilename = _ui->keyfilePathComboBrowse->text();
    }
    Q_ASSERT(!tarsnapKeyFilename.isEmpty());

    settings.setValue("tarsnap/key", tarsnapKeyFilename);
    settings.setValue("tarsnap/user", _ui->tarsnapUserLineEdit->text());

    if(checkComplete())
    {
        _registering = Yes;
        // Display message after checkComplete() clears the label.
        if(!useExistingKeyfile)
            _ui->statusLabel->messageNormal("Generating keyfile...");
        else
            _ui->statusLabel->messageNormal("Verifying archive integrity...");
        // Request that the backend does the operation.
        emit registerMachineRequested(_ui->tarsnapPasswordLineEdit->text(),
                                      _ui->machineNameLineEdit->text(),
                                      useExistingKeyfile);
    }
}

void RegisterPage::registerMachineResponse(TaskStatus     status,
                                           const QString &reason)
{
    TSettings settings;

    // Get keyfile and sanity check.
    QString tarsnapKeyFilename = settings.value("tarsnap/key", "").toString();
    if((status == TaskStatus::Completed) && (tarsnapKeyFilename.isEmpty()))
    {
        // This should never happen.
        status = TaskStatus::Failed;
        reportError("No keyfile set");
    }

    switch(status)
    {
    case TaskStatus::Completed:
        _registering = Done;
        _ui->statusLabel->clear();
        updateLoadingAnimation(true);
        next();
        break;
    case TaskStatus::Failed:
        _registering = No;
        reportError(reason);
        checkComplete();
        break;
    default:
        // We shouldn't receive anything else, so ignore it.
        break;
    }
}

bool RegisterPage::checkCreateKeyfile()
{
    if(_ui->machineNameLineEdit->text().isEmpty())
        return setProceedButton(false);

    if(_ui->tarsnapUserLineEdit->text().isEmpty())
        return setProceedButton(false);

    if(_ui->tarsnapPasswordLineEdit->text().isEmpty())
        return setProceedButton(false);

    return true;
}

bool RegisterPage::checkUseKeyfile()
{
    const QString filename = _ui->keyfilePathComboBrowse->text();

    const QString errorMsg = validate_readable_file(filename);
    if(!errorMsg.isEmpty())
        return reportError("", _ui->keyfilePathComboBrowse, errorMsg);

    // File is ok.
    _ui->keyfilePathComboBrowse->setStatusOk("");
    return true;
}

void RegisterPage::updateLoadingAnimation(bool idle)
{
    _ui->busyLabel->animate(!idle);
}

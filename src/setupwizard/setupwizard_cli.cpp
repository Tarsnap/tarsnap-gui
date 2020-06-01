#include "setupwizard/setupwizard_cli.h"

WARNINGS_DISABLE
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QStandardPaths>
#include <QWidget>

#include "ui_setupwizard_cli.h"
WARNINGS_ENABLE

#include "TElidedLabel.h"
#include "TOkLabel.h"
#include "TPathLineBrowse.h"
#include "TSettings.h"
#include "TWizardPage.h"

#include "messages/taskstatus.h"

#include "dir-utils.h"
#include "tasks/tasks-defs.h"

CliPage::CliPage(QWidget *parent)
    : TWizardPage(parent), _ui(new Ui::CliPage), _problemOccurred(false)
{
    _ui->setupUi(this);

    // We only want to expand the widget if there's a problem.
    _ui->detailsWidget->hide();
    _ui->detailsButton->setChecked(false);

    // Basic operation on this page.
    connect(_ui->detailsButton, &QPushButton::toggled, _ui->detailsWidget,
            &QWidget::setVisible);

    // A config field changed.
    connect(_ui->cliPathLineBrowse, &TPathLineBrowse::textChanged, this,
            &CliPage::tarsnapPathChanged);
    connect(_ui->cachePathLineBrowse, &TPathLineBrowse::textChanged, this,
            &CliPage::tarsnapCacheChanged);
    connect(_ui->appdataPathLineBrowse, &TPathLineBrowse::textChanged, this,
            &CliPage::appDataDirChanged);
}

CliPage::~CliPage()
{
    delete _ui;
}

void CliPage::initializePage()
{
    TWizardPage::initializePage();

    // CLI path.
    struct DirMessage result      = findTarsnapClientInPath("", true);
    QString           tarsnapPath = result.dirname;
    _ui->cliPathLineBrowse->setText(tarsnapPath);
    if(tarsnapPath.isEmpty())
    {
        // Trigger this manually, because the automatic connection
        // won't trigger if it's blank.
        tarsnapPathChanged(tarsnapPath);
    }

    /// Cache dir.
    QString tarsnapCacheDir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if(ensureDirExists(tarsnapCacheDir))
        _ui->cachePathLineBrowse->setText(tarsnapCacheDir);

    // App Data dir.
    QString appDataDir = QStandardPaths::writableLocation(APPDATA);
    if(ensureDirExists(appDataDir))
        _ui->appdataPathLineBrowse->setText(appDataDir);
}

bool CliPage::checkComplete()
{
    // Check the remaining items.
    TSettings settings;

    // Do we have a valid version number?  (This is the last piece of config.)
    if(!settings.contains("tarsnap/version"))
        return setProceedButton(false);

    // Do we have everything else?  (Double-check; almost certainly not needed.)
    if((!settings.contains("tarsnap/path"))
       || (!settings.contains("tarsnap/cache"))
       || (!settings.contains("app/app_data")))
        return setProceedButton(false);

    // We're ok.
    _ui->validationLabel->messageNormal(_successMessage);
    return setProceedButton(true);
}

bool CliPage::reportError(const QString &text, TPathLineBrowse *plb,
                          const QString &plb_text)
{
    // General display.
    _ui->validationLabel->messageError(text);

    // Handle widget-specific display.
    if(plb != nullptr)
        plb->setStatusError(plb_text);

    // Generic handling of errors.
    _ui->detailsButton->setChecked(true);
    setProceedButton(false);
    _problemOccurred = true;
    return false;
}

bool CliPage::tarsnapCacheChanged(const QString &text)
{
    // Clear previous setting.
    TSettings settings;
    settings.remove("tarsnap/cache");

    // Validate the directory and display any errors.
    const QString errorMsg = validate_writeable_dir(text);
    if(!errorMsg.isEmpty())
        return reportError("", _ui->cachePathLineBrowse, errorMsg);

    // We're ok.
    const QString pathname = QFileInfo(text).canonicalFilePath();
    settings.setValue("tarsnap/cache", pathname);
    _ui->cachePathLineBrowse->setStatusOk("");
    checkComplete();
    return true;
}

bool CliPage::appDataDirChanged(const QString &text)
{
    // Clear previous setting.
    TSettings settings;
    settings.remove("app/app_data");

    // Validate the directory and display any errors.
    const QString errorMsg = validate_writeable_dir(text);
    if(!errorMsg.isEmpty())
        return reportError("", _ui->appdataPathLineBrowse, errorMsg);

    // We're ok.
    const QString pathname = QFileInfo(text).canonicalFilePath();
    settings.setValue("app/app_data", pathname);
    _ui->appdataPathLineBrowse->setStatusOk("");
    checkComplete();
    return true;
}

bool CliPage::tarsnapPathChanged(const QString &text)
{
    // Clear previous settings.
    TSettings settings;
    settings.remove("tarsnap/path");
    settings.remove("tarsnap/version");

    // Look for the CLI binaries and display any errors.
    struct DirMessage result     = findTarsnapClientInPath(text, true);
    QString           tarsnapDir = result.dirname;
    if(tarsnapDir.isEmpty())
    {
        Q_ASSERT(result.errorMessage != "");
        return reportError(
            tr("Visit <a href=\"https://tarsnap.com\">tarsnap.com</a>"
               " to acquire the command-line utilities."),
            _ui->cliPathLineBrowse, result.errorMessage);
    }

    // We're ok.
    settings.setValue("tarsnap/path", tarsnapDir);
    emit tarsnapVersionRequested();
    checkComplete();
    return true;
}

void CliPage::tarsnapVersionResponse(TaskStatus     status,
                                     const QString &versionString)
{
    TSettings settings;

    // Sanity check.
    if(versionString.isEmpty())
        status = TaskStatus::Failed;

    // Handle response.
    switch(status)
    {
    case TaskStatus::Completed:
        // Record value
        settings.setValue("tarsnap/version", versionString);
        // Auto-focus on "Next" if the user isn't manually changing things.
        if(checkComplete() && !_problemOccurred)
            _ui->nextButton->setFocus();
        // Display message (after checkComplete, which can clear the label).
        _ui->validationLabel->messageNormal(
            tr("Tarsnap CLI version ") + versionString + tr(" detected.") + " "
            + TOkLabel::getRichText(TOkLabel::Ok));
        // Save the message, allowing us to return to it if we
        // temporarily disable completion (e.g., after fiddling with dirs).
        _successMessage = _ui->validationLabel->text();
        _ui->cliPathLineBrowse->setStatusOk("");
        break;
    case TaskStatus::VersionTooLow:
        // Don't record the too-low version number.
        reportError(
            tr("Tarsnap CLI version ") + versionString
                + tr(" too low; must be at least %1").arg(TARSNAP_MIN_VERSION),
            _ui->cliPathLineBrowse, tr("Version too low."));
        break;
    case TaskStatus::Failed:
        reportError("", _ui->cliPathLineBrowse,
                    tr("Error retrieving Tarsnap CLI verison"));
        break;
    default:
        break;
    }
}

bool CliPage::ensureDirExists(const QString &dirname)
{
    // Create directory (if needed).
    if(QDir().mkpath(dirname))
        return true;
    else
    {
        QMessageBox::critical(this, tr("Could not create directory"),
                              tr("Could not create directory") + ": "
                                  + dirname);
        return false;
    }
}

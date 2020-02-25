#include "setupwizard_cli.h"

WARNINGS_DISABLE
#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <QStandardPaths>

#include "ui_setupwizard_cli.h"
WARNINGS_ENABLE

#include <OkLabel.h>
#include <TSettings.h>
#include <TWizardPage.h>

#include "tasks-defs.h"
#include "taskstatus.h"
#include "utils.h"

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
    connect(_ui->cliPathLineBrowse, &PathLineBrowse::textChanged, this,
            &CliPage::tarsnapPathChanged);
    connect(_ui->cachePathLineBrowse, &PathLineBrowse::textChanged, this,
            &CliPage::tarsnapCacheChanged);
    connect(_ui->appdataPathLineBrowse, &PathLineBrowse::textChanged, this,
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
    struct DirMessage result      = Utils::findTarsnapClientInPath("", true);
    QString           tarsnapPath = result.dirname;
    _ui->cliPathLineBrowse->setText(tarsnapPath);
    if(tarsnapPath.isEmpty())
    {
        // Trigger this manually, because the automatic connection
        // won't trigger it's blank.
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

    _ui->validationLabel->setStyleSheet("");
    _ui->validationLabel->setText(_successMessage);
    return setProceedButton(true);
}

bool CliPage::reportError(const QString &text, PathLineBrowse *plb,
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

    if(text.isEmpty())
        return reportError(tr("Empty Tarsnap cache directory set."));

    QString tarsnapCacheDir = Utils::validateTarsnapCache(text);
    if(tarsnapCacheDir.isEmpty())
        return reportError(tr("Invalid Tarsnap cache directory set."));

    // We're ok
    settings.setValue("tarsnap/cache", tarsnapCacheDir);
    checkComplete();
    return true;
}

bool CliPage::appDataDirChanged(const QString &text)
{
    // Clear previous setting.
    TSettings settings;
    settings.remove("app/app_data");

    if(text.isEmpty())
        return reportError(tr("Empty App data directory set."));

    QString appDataDir = Utils::validateAppDataDir(text);
    if(appDataDir.isEmpty())
        return reportError(tr("Invalid App data directory set."));

    // We're ok
    settings.setValue("app/app_data", appDataDir);
    checkComplete();
    return true;
}

bool CliPage::tarsnapPathChanged(const QString &text)
{
    // Clear previous settings.
    TSettings settings;
    settings.remove("tarsnap/path");
    settings.remove("tarsnap/version");

    // Don't check for an empty dir here, because we want users to see the
    // "visit tarsnap.com" message if they don't have the binaries.

    struct DirMessage result     = Utils::findTarsnapClientInPath(text, true);
    QString           tarsnapDir = result.dirname;
    if(tarsnapDir.isEmpty())
        return reportError(tr("Tarsnap utilities not found. Visit "
                              "<a href=\"https://tarsnap.com\">tarsnap.com</a>"
                              " for help with acquiring them."));

    // We're ok
    settings.setValue("tarsnap/path", tarsnapDir);
    emit tarsnapVersionRequested();
    checkComplete();
    return true;
}

void CliPage::tarsnapVersionResponse(TaskStatus status, QString versionString)
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
            + OkLabel::getRichText(OkLabel::Ok));
        // Save the message, allowing us to return to it if we
        // temporarily disable completion (e.g., after fiddling with dirs).
        _successMessage = _ui->validationLabel->text();
        break;
    case TaskStatus::VersionTooLow:
        // Don't record the too-low version number.
        reportError(
            tr("Tarsnap CLI version ") + versionString
            + tr(" too low; must be at least %1").arg(TARSNAP_MIN_VERSION));
        break;
    case TaskStatus::Failed:
        reportError(tr("Error retrieving Tarsnap CLI verison"));
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

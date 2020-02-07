#include "setupwizard.h"

WARNINGS_DISABLE
#include <QPixmap>
#include <QString>
WARNINGS_ENABLE

#include <TSettings.h>
#include <TWizardPage.h>

#include "setupwizard_cli.h"
#include "setupwizard_final.h"
#include "setupwizard_intro.h"
#include "setupwizard_register.h"

#include "taskstatus.h"

SetupWizard::SetupWizard(QWidget *parent) : TWizard(parent)
{
    // This would normally be in a .ui file, but that's the only piece
    // of UI config, so it seems overkill to add a file for just that.
    setLogo(QPixmap(":/logos/tarsnap-icon-h32.png"));

    // Actual pages
    _introPage    = new IntroPage();
    _cliPage      = new CliPage();
    _registerPage = new RegisterPage();
    _finalPage    = new FinalPage();
    addPages(QList<TWizardPage *>()
             << _introPage << _cliPage << _registerPage << _finalPage);

    // Frontend-Backend communication
    connect(_cliPage, &CliPage::tarsnapVersionRequested, this,
            &SetupWizard::tarsnapVersionRequested);
    connect(_registerPage, &RegisterPage::registerMachineRequested, this,
            &SetupWizard::registerMachineRequested);
}

SetupWizard::~SetupWizard()
{
}

void SetupWizard::tarsnapVersionResponse(TaskStatus status,
                                         QString    versionString)
{
    _cliPage->tarsnapVersionResponse(status, versionString);
}

void SetupWizard::registerMachineResponse(TaskStatus status, QString reason)
{
    _registerPage->registerMachineResponse(status, reason);
}

void SetupWizard::updateLoadingAnimation(bool idle)
{
    TWizardPage *page = currentPage();
    if(page == _registerPage)
        _registerPage->updateLoadingAnimation(idle);

    // Other pages: we expect to see two such messages on CliPage (due
    // to tarsnapVersionRequested / tarsnapVersionResponse), and will
    // likely see one on FinalPage (when TaskManager disables the
    // "active task" from registration, and we move to the next page
    // before that signal arrives).
}

void SetupWizard::finishWizard()
{
    // We've either completed the setup wizard, or deliberately skipped it.
    TSettings settings;
    settings.setValue("app/wizard_done", true);

    // Call parent function
    TWizard::accept();
}

void SetupWizard::reject()
{
    // Remove any saved settings
    TSettings settings;
    settings.clear();

    // Call parent function
    TWizard::reject();
}

void SetupWizard::skipWizard()
{
    // Remove any saved settings
    TSettings settings;
    settings.clear();

    finishWizard();
}

#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

#include "TWizard.h"

#include "taskstatus.h"

/* Forward declarations. */
class IntroPage;
class CliPage;
class RegisterPage;
class FinalPage;

/**
 * \ingroup widgets-setup
 * \brief The SetupWizard is a QWizard which guides the user
 * through configuring the essential options of this app.
 */
class SetupWizard : public TWizard
{
    Q_OBJECT

public:
    //! Constructor.
    SetupWizard(QWidget *parent = nullptr);
    ~SetupWizard() override;

public slots:
    //! The TaskManager has finished determining the version of the CLI app.
    //! \param status did the task complete?
    //! \param versionString the version number.
    void tarsnapVersionResponse(TaskStatus status, QString versionString);
    //! The TaskManager has finished attempting to register the machine.
    //! \param status either \c TaskStatus::Completed or
    //!                \c TaskStatus::Failed.
    //! \param reason text which explains why the registration failed.
    void registerMachineResponse(TaskStatus status, QString reason);
    //! The TaskManager is indicating that it is performing a background task.
    //! \param idle no task is running.
    void updateLoadingAnimation(bool idle);

    //! Quit the wizard, clear any config, allow re-running the setup.
    void reject() override;

signals:
    //! We need to determine the Tarsnap CLI version number.
    void tarsnapVersionRequested();
    //! We need to register this machine with the Tarsnap service.
    //! \param password the user's Tarsnap service password.
    //! \param useExistingKeyfile should we use an existing keyfile?
    void registerMachineRequested(QString password, bool useExistingKeyfile);

protected slots:
    //! Quit the wizard, clear any config, don't run setup again.
    void skipWizard() override;
    //! Quit the wizard, save the config, don't run setup again.
    void finishWizard() override;

private:
    IntroPage *   _introPage;
    CliPage *     _cliPage;
    RegisterPage *_registerPage;
    FinalPage *   _finalPage;
};

#endif /* !SETUPWIZARD_H */

#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "TWizard.h"

#include "messages/taskstatus.h"

/* Forward declaration(s). */
class IntroPage;
class CliPage;
class RegisterPage;
class FinalPage;
class QWidget;

/**
 * \ingroup widgets-setup
 * \brief The SetupWizard is a TWizard which guides the user
 * through configuring the essential options of this app.
 */
class SetupWizard : public TWizard
{
    Q_OBJECT

public:
    //! Constructor.
    SetupWizard(QWidget *parent = nullptr);

public slots:
    //! The TaskManager has finished determining the version of the CLI app.
    //! \param status did the task complete?
    //! \param versionString the version number.
    void tarsnapVersionResponse(TaskStatus     status,
                                const QString &versionString);
    //! The TaskManager has finished attempting to register the machine.
    //! \param status either \c TaskStatus::Completed or
    //!                \c TaskStatus::Failed.
    //! \param reason text which explains why the registration failed.
    void registerMachineResponse(TaskStatus status, const QString &reason);
    //! The TaskManager has a progress message from --fsck.
    //! \param stdOut line(s) from stdout of tarsnap --fsck
    void registerMachineProgress(const QString &stdOut);
    //! Update the number of tasks.
    void updateNumTasks(bool backupRunning, int numRunning, int numQueued);

    //! Quit the wizard, clear any config, allow re-running the setup.
    void reject() override;

signals:
    //! We need to determine the Tarsnap CLI version number.
    void tarsnapVersionRequested();
    //! We need to register this machine with the Tarsnap service.
    //! \param password the user's Tarsnap service password.
    //! \param machine the user's machine name.
    //! \param useExistingKeyfile should we use an existing keyfile?
    void registerMachineRequested(const QString &password,
                                  const QString &machine,
                                  const bool     useExistingKeyfile);

protected slots:
    //! Quit the wizard, clear any config, don't run setup again.
    void skipWizard() override;
    //! Quit the wizard, save the config, don't run setup again.
    void finishWizard() override;

private:
    IntroPage    *_introPage;
    CliPage      *_cliPage;
    RegisterPage *_registerPage;
    FinalPage    *_finalPage;
};

#endif /* !SETUPWIZARD_H */

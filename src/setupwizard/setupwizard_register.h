#ifndef SETUPWIZARD_REGISTER_H
#define SETUPWIZARD_REGISTER_H

#include <TWizardPage.h>

#include "taskstatus.h"

namespace Ui
{
class RegisterPage;
}

/**
 * \ingroup widgets-setup
 * \brief The RegisterPage is a TWizardPage which generates a
 * keyfile or runs fsck.
 */
class RegisterPage : public TWizardPage
{
    Q_OBJECT

public:
    //! Constructor.
    RegisterPage(QWidget *parent = nullptr);
    ~RegisterPage() override;

    //! Initialize the page when it first becomes visible.
    void initializePage() override;

#ifdef QT_TESTLIB_LIB
    Ui::RegisterPage *get_ui() const { return _ui; }
#endif

public slots:
    //! The TaskManager has finished attempting to register the machine.
    //! \param status either \c TaskStatus::Completed or
    //!                \c TaskStatus::Failed.
    //! \param reason text which explains why the registration failed.
    void registerMachineResponse(TaskStatus status, QString reason);
    //! The TaskManager is indicating that it is performing a background task.
    //! \param idle no task is running.
    void updateLoadingAnimation(bool idle);

signals:
    //! We need to register this machine with the Tarsnap service.
    //! \param password the user's Tarsnap service password.
    //! \param machine the user's machine name.
    //! \param useExistingKeyfile should we use an existing keyfile?
    void registerMachineRequested(const QString &password,
                                  const QString &machine,
                                  const bool     useExistingKeyfile);

protected:
    //! Has the user filled in all mandatory data on the page?
    bool checkComplete() override;

    //! User activated the "next" button.
    void next() override;

private slots:
    void createKeyfile();
    void useExistingKeyfile();

    void machineKeyChanged(const QString &text);

private:
    Ui::RegisterPage *_ui;

    bool _createKey;

    void reportError(const QString &text);

    enum
    {
        No,
        Yes,
        Done
    } _registering;

    void registerMachine();
};

#endif /* !SETUPWIZARD_REGISTER_H */

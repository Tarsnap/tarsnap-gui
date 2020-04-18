#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QMovie>
WARNINGS_ENABLE

#include <messages/taskstatus.h>

namespace Ui
{
class SetupDialog;
}
class QAbstractButton;

/*!
 * \ingroup widgets-specialized
 * \brief The SetupDialog is a QDialog which guides the user
 * through configuring the essential options of this app.
 */
class SetupDialog : public QDialog
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSetupWizard;
#endif

public:
    /// Constructor.
    explicit SetupDialog(QWidget *parent = nullptr);
    ~SetupDialog();

#ifdef QT_TESTLIB_LIB
    enum Button
    {
        SkipButton,
        BackButton,
        NextButton,
        FinishButton
    };
    /// Get the indicated button
    QAbstractButton *button(enum SetupDialog::Button which);
#endif

public slots:
    /// The TaskManager has finished attempting to register the machine.
    /// \param status either \c TaskStatus::Completed or
    ///                \c TaskStatus::Failed.
    /// \param reason text which explains why the registration failed.
    void registerMachineResponse(TaskStatus status, QString reason);
    /// The TaskManager has finished determining the version of the CLI app.
    /// \param status did the task complete?
    /// \param versionString the version number.
    void tarsnapVersionResponse(TaskStatus status, QString versionString);
    /// The TaskManager is indicating that it is performing a background task.
    /// \param idle no task is running.
    void updateLoadingAnimation(bool idle);

#ifdef QT_TESTLIB_LIB
    /// Get the ui object.
    Ui::SetupDialog *get_ui();
    /// Get the current page title.
    QString pageTitle() const;
#endif

signals:
    /// We need to register this machine with the Tarsnap service.
    /// \param password the user's Tarsnap service password.
    /// \param useExistingKeyfile should we use an existing keyfile?
    void registerMachineRequested(QString password, bool useExistingKeyfile);
    /// We need to determine the Tarsnap CLI version number.
    void tarsnapVersionRequested();

private slots:
    void wizardPageChanged(int);
    void showTarsnapPathBrowse();
    void showTarsnapCacheBrowse();
    void showAppDataBrowse();
    bool validateCLIPage();
    void createKeyfile();
    void useExistingKeyfile();
    bool validateRegisterPage();
    void registerHaveKeyBrowse();
    void backButtonClicked();
    void nextButtonClicked();
    void reject();

private:
    Ui::SetupDialog *_ui;

    void setNextPage();
    void registerMachine();
    void finishedWizard();

    void initCLIPage();
    void initRegisterPage();
    void skipWizard();
};

#endif // SETUPDIALOG_H

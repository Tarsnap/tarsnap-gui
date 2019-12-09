#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QMovie>
WARNINGS_ENABLE

#include <taskstatus.h>

namespace Ui
{
class SetupDialog;
}

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

signals:
    /// We need to register this machine with the Tarsnap service.
    /// \param password the user's Tarsnap service password.
    /// \param useExistingKeyfile should we use an existing keyfile?
    void registerMachineRequested(QString password, bool useExistingKeyfile);
    /// We need to determine the Tarsnap CLI version number.
    void tarsnapVersionRequested();

private slots:
    void wizardPageChanged(int);
    void skipToPage();
    void showTarsnapPathBrowse();
    void showTarsnapCacheBrowse();
    void showAppDataBrowse();
    bool validateAdvancedSetupPage();
    void restoreNo();
    void restoreYes();
    bool validateRegisterPage();
    void registerHaveKeyBrowse();
    void backButtonClicked();
    void nextButtonClicked();

private:
    Ui::SetupDialog *_ui;

    QString _tarsnapCacheDir;

    void setNextPage();
    void registerMachine();
    void finishedWizard();

    void initCLIPage();
};

#endif // SETUPDIALOG_H

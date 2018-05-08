#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "ui_setupdialog.h"
#include <taskmanager.h>

#include <QDialog>
#include <QMovie>

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
    /// \param status: either \c TaskStatus::Completed or
    ///                \c TaskStatus::Failed.
    /// \param reason: text which explains why the registration failed.
    void registerMachineStatus(TaskStatus status, QString reason);
    /// The TaskManager has finished determining the version of the CLI app.
    /// \param versionString: the version number.
    void setTarsnapVersion(QString versionString);
    /// The TaskManager is indicating that it is performing a background task.
    /// \param idle: no task is running.
    void updateLoadingAnimation(bool idle);

signals:
    /// We need to register this machine with the Tarsnap service.
    /// \param user: the email address of the user.
    /// \param password: the user's Tarsnap service password.
    /// \param machine: the name of this machine.
    /// \param key: the keyfile; might already exist, or might be new.
    /// \param tarsnapPath: the path of the CLI app.
    /// \param cachePath: the Tarsnap cachedir.
    void requestRegisterMachine(QString user, QString password, QString machine,
                                QString key, QString tarsnapPath,
                                QString cachePath);
    /// We need to determine the Tarsnap CLI version number.
    /// \param tarsnapPath: the path of the CLI app.
    void getTarsnapVersion(QString tarsnapPath);
    /// Request cache initialization (used when generating a new key)
    void initializeCache();

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
    Ui::SetupDialog _ui;
    QPoint          _windowDragPos;
    QMovie          _loadingAnimation;

    QString _tarsnapDir;
    QString _tarsnapVersion;
    QString _tarsnapCacheDir;
    QString _appDataDir;
    QString _tarsnapKeyFile;

    void setNextPage();
    void registerMachine();
    void commitSettings(bool skipped = false);
};

#endif // SETUPDIALOG_H

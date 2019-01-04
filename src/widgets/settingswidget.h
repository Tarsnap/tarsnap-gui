#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "confirmationdialog.h"
#include "scheduling.h"
#include "tarsnapaccount.h"
#include "taskmanager.h"
#include "translator.h"

#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

namespace Ui
{
class SettingsWidget;
}

/*!
 * \ingroup widgets-main
 * \brief The SettingsWidget is a QWidget which is handles configuration.
 */
class SettingsWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSettingsWidget;
#endif

public:
    //! Constructor.
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

    // For the Account tab
    //! Make TarsnapAccount update the account info from the Tarsnap servers.
    void getAccountInfo();

    // For the Application tab
    //! Update the number of tasks;
    void updateNumTasks(int numRunning, int numQueued);

public slots:
    //! Initialization routines.
    void initializeSettingsWidget();

    // For the Account tab
    //! Update the global Tarsnap --print-stats values in the Settings tab.
    void overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
                             quint64 sizeUniqueTotal,
                             quint64 sizeUniqueCompressed, quint64 archiveCount);
    //! Save the Tarsnap key ID.
    void saveKeyId(QString key_filename, quint64 id);

    //! For the Application tab
    //! Update the Tarsnap version number, and store it in the settings.
    void updateTarsnapVersion(QString versionString);

signals:
    // For the Account tab
    //! The confirmation has been accepted and the timer has finished; go
    //! ahead and nuke all archives.
    void nukeArchives();
    //! Display a message.
    void newStatusMessage(QString message, QString detail);
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(QString key_filename);

    // For the Backup tab
    //! Show the simulation status
    void newSimulationStatus(int state);

    // For the Application tab
    //! Begin tarsnap --fsck or --fsck-prune
    void repairCache(bool prune);
    //! Clear all Journal entries.
    void clearJournal();
    //! The user has confirmed that the settings can be wiped; go ahead
    //! and run the setup wizard again.
    void runSetupWizard();
    //! Begin tarsnap --version
    void getTarsnapVersion(QString tarsnapPath);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private slots:
    // For the Account tab
    void commitSettings();
    void updateAccountCredit(qreal credit, QDate date);
    void updateLastMachineActivity(QStringList activityFields);
    bool validateMachineKeyPath();
    void accountMachineUseHostnameButtonClicked();
    void accountMachineKeyBrowseButtonClicked();
    void nukeArchivesButtonClicked();
    void nukeConfirmed();
    void nukeCancelled();
    // For the Backup tab
    void updateSimulationIcon(int state);
    void enableJobSchedulingButtonClicked();
    void disableJobSchedulingButtonClicked();
    // For the Application tab
    void clearJournalClicked();
    bool validateTarsnapPath();
    bool validateTarsnapCache();
    bool validateAppDataDir();
    void tarsnapPathBrowseButtonClicked();
    void tarsnapCacheBrowseButton();
    void appDataButtonClicked();
    void runSetupWizardClicked();
    void downloadsDirBrowseButtonClicked();

private:
    Ui::SettingsWidget *_ui;

    void updateUi();
    void loadSettings();

    // For the Account tab
    ConfirmationDialog _nukeConfirmationDialog;
    TarsnapAccount     _tarsnapAccount;

    // For the Application tab
    int _runningTasks;
    int _queuedTasks;
};

#endif // SETTINGSWIDGET_H

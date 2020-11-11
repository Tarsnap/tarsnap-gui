#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDate>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>
WARNINGS_ENABLE

#include "messages/taskstatus.h"

/* Forward declaration(s). */
namespace Ui
{
class SettingsWidget;
}
class ConfirmationDialog;
class QEvent;
class SchedulingWidgets;
class TarsnapAccountDialog;

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
    ~SettingsWidget() override;

    // For the Account tab
    //! Make TarsnapAccount update the account info from the Tarsnap servers.
    void getAccountInfo();

    //! Update the archive count.
    void setArchiveCount(quint64 archiveCount);

    // For the Application tab
    //! Update the number of tasks;
    void updateNumTasks(int numRunning, int numQueued);

public slots:
    //! Initialization routines.
    void initializeSettingsWidget();

    // For the Account tab
    //! Save the Tarsnap key ID.
    void saveKeyId(const QString &key_filename, quint64 id);

    //! For the Application tab
    //! Update the Tarsnap version number, and store it in the settings.
    void tarsnapVersionResponse(TaskStatus     status,
                                const QString &versionString);

signals:
    // For the Account tab
    //! The confirmation has been accepted and the timer has finished; go
    //! ahead and nuke all archives.
    void nukeArchives();
    //! Display a message.
    void newStatusMessage(const QString &message, const QString &detail);
    //! Begin tarsnap-keymgmt --print-key-id \<key_filename\>
    void getKeyId(const QString &key_filename);

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
    void tarsnapVersionRequested(const QString &tarsnapPath);
    //! The "app/iec_prefixes" has changed.
    void iecChanged();
    //! Begin tarsnap --list-archives
    void getArchives();

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private slots:
    // For the Account tab
    void updateAccountCredit(qreal credit, QDate date);
    void updateLastMachineActivity(const QStringList &activityFields);
    bool validateMachineKeyPath();
    void accountMachineKeyBrowseButtonClicked();
    void nukeArchivesButtonClicked();
    void nukeConfirmed();
    void nukeCancelled();
    // For the Backup tab
    void updateSimulationIcon(int state);
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

    void updateKeyboardShortcutInfo();
    void loadSettings();

    void initSettingsSetValue();

    // For the Account tab
    ConfirmationDialog *_nukeConfirmationDialog;

    TarsnapAccountDialog *_tarsnapAccountDialog;

    SchedulingWidgets *_schedulingWidgets;

    QString _archiveCountStr;

    // For the Application tab
    int _runningTasks;
    int _queuedTasks;
};

#endif // SETTINGSWIDGET_H

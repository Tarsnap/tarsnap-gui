#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "scheduling.h"
#include "tarsnapaccount.h"
#include "taskmanager.h"
#include "ui_settingswidget.h"

#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

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

    // For the Account tab
    void getAccountInfo();

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

private slots:
    // For the Account tab
    void commitSettings();
    void updateAccountCredit(qreal credit, QDate date);
    void updateLastMachineActivity(QStringList activityFields);
    bool validateMachineKeyPath();
    void accountMachineUseHostnameButtonClicked();
    void accountMachineKeyBrowseButtonClicked();
    void nukeTimerFired();
    void nukeArchivesButtonClicked();
    // For the Backup tab
    void updateSimulationIcon(int state);
    void enableJobSchedulingButtonClicked();
    void disableJobSchedulingButtonClicked();

private:
    Ui::SettingsWidget _ui;

    void updateUi();
    void loadSettings();

    // For the Account tab
    QTimer         _nukeTimer;
    int            _nukeTimerCount;
    QMessageBox    _nukeCountdown;
    TarsnapAccount _tarsnapAccount;
    QInputDialog   _nukeInput;
};

#endif // SETTINGSWIDGET_H

#include "app-gui.h"
#include "debug.h"
#include "init-shared.h"
#include "translator.h"
#include "utils.h"
#include "widgets/setupdialog.h"

#include <assert.h>

#include <QFontDatabase>
#include <QMessageBox>

AppGui::AppGui(int &argc, char **argv, struct optparse *opt)
    : QApplication(argc, argv), _mainWindow(nullptr), _notification()
{
    // Sanity check
    assert(opt != NULL);

    // Get values from optparse.  The (x == 1) is probably unnecessary, but
    // better safe than sorry!
    _jobsOption  = (opt->jobs == 1);
    _checkOption = (opt->check == 1);
    _configDir   = opt->config_dir;

    init_shared(this);

    setQuitOnLastWindowClosed(false);

    qRegisterMetaType<QSystemTrayIcon::ActivationReason>(
        "QSystemTrayIcon::ActivationReason");
}

AppGui::~AppGui()
{
    if(_mainWindow)
        delete _mainWindow;
    _managerThread.quit();
    _managerThread.wait();
}

bool AppGui::initializeCore()
{
    struct init_info info = init_shared_core(this, _configDir);

    if(info.status == INIT_NEEDS_SETUP)
        return runSetupWizard();
    else if(info.status == INIT_DRY_RUN)
    {
        QMessageBox::warning(nullptr, tr("Tarsnap warning"), info.message);
        // There's no point trying to automatically process jobs.
        if(_jobsOption)
            return false;
    }

    if(_jobsOption || _checkOption)
    {
        if(info.status == INIT_SCHEDULE_OK)
            DEBUG << info.extra;
        else if(info.status == INIT_SCHEDULE_ERROR)
        {
            DEBUG << info.message;
            return false;
        }

        // We don't have anything else to do
        if(_checkOption)
            return true;
    }

    if(!_jobsOption)
    {
        if(info.status == INIT_SCHEDULE_OK)
            QMessageBox::information(nullptr, tr("Updated OS X launchd path"),
                                     info.message);
        else if(info.status == INIT_SCHEDULE_ERROR)
            QMessageBox::information(nullptr,
                                     tr("Failed to updated OS X launchd path"),
                                     info.message);
    }
    return true;
}

bool AppGui::prepMainLoop()
{
    // Nothing to do.
    if(_checkOption)
        return false;

    // Initialize the PersistentStore early
    PersistentStore::instance();

    // Queue loading the journal when we have an event loop.
    QMetaObject::invokeMethod(&_journal, "load", QUEUED);

    connect(&_taskManager, &TaskManager::displayNotification, &_notification,
            &Notification::displayNotification, QUEUED);
    connect(&_taskManager, &TaskManager::message, &_journal, &Journal::log,
            QUEUED);

    if(_jobsOption)
    {
        setQuitLockEnabled(true);
        connect(&_notification, &Notification::activated, this,
                &AppGui::showMainWindow, QUEUED);
        connect(&_notification, &Notification::messageClicked, this,
                &AppGui::showMainWindow, QUEUED);
        QMetaObject::invokeMethod(&_taskManager, "runScheduledJobs", QUEUED);
    }
    else
    {
        showMainWindow();
    }

    return true;
}

void AppGui::showMainWindow()
{
    if(_mainWindow != nullptr)
        return;

    setQuitLockEnabled(false);
    disconnect(&_notification, &Notification::activated, this,
               &AppGui::showMainWindow);
    disconnect(&_notification, &Notification::messageClicked, this,
               &AppGui::showMainWindow);

    _mainWindow = new MainWindow();
    Q_ASSERT(_mainWindow != nullptr);

    connect(_mainWindow, &MainWindow::getTarsnapVersion, &_taskManager,
            &TaskManager::getTarsnapVersion, QUEUED);
    connect(&_taskManager, &TaskManager::tarsnapVersion, _mainWindow,
            &MainWindow::updateTarsnapVersion, QUEUED);
    connect(_mainWindow, &MainWindow::backupNow, &_taskManager,
            &TaskManager::backupNow, QUEUED);
    connect(_mainWindow, &MainWindow::getArchives, &_taskManager,
            &TaskManager::getArchives, QUEUED);
    connect(&_taskManager, &TaskManager::archiveList, _mainWindow,
            &MainWindow::archiveList, QUEUED);
    connect(&_taskManager, &TaskManager::addArchive, _mainWindow,
            &MainWindow::addArchive, QUEUED);
    connect(_mainWindow, &MainWindow::deleteArchives, &_taskManager,
            &TaskManager::deleteArchives, QUEUED);
    connect(_mainWindow, &MainWindow::loadArchiveStats, &_taskManager,
            &TaskManager::getArchiveStats, QUEUED);
    connect(_mainWindow, &MainWindow::loadArchiveContents, &_taskManager,
            &TaskManager::getArchiveContents, QUEUED);
    connect(&_taskManager, &TaskManager::idle, _mainWindow,
            &MainWindow::updateLoadingAnimation, QUEUED);
    connect(_mainWindow, &MainWindow::getOverallStats, &_taskManager,
            &TaskManager::getOverallStats, QUEUED);
    connect(&_taskManager, &TaskManager::overallStats, _mainWindow,
            &MainWindow::overallStatsChanged, QUEUED);
    connect(_mainWindow, &MainWindow::repairCache, &_taskManager,
            &TaskManager::fsck, QUEUED);
    connect(_mainWindow, &MainWindow::nukeArchives, &_taskManager,
            &TaskManager::nuke, QUEUED);
    connect(_mainWindow, &MainWindow::restoreArchive, &_taskManager,
            &TaskManager::restoreArchive, QUEUED);
    connect(_mainWindow, &MainWindow::runSetupWizard, this, &AppGui::reinit,
            QUEUED);
    connect(_mainWindow, &MainWindow::stopTasks, &_taskManager,
            &TaskManager::stopTasks, QUEUED);
    connect(&_taskManager, &TaskManager::jobList, _mainWindow,
            &MainWindow::jobList, QUEUED);
    connect(_mainWindow, &MainWindow::deleteJob, &_taskManager,
            &TaskManager::deleteJob, QUEUED);
    connect(_mainWindow, &MainWindow::getTaskInfo, &_taskManager,
            &TaskManager::getTaskInfo, QUEUED);
    connect(&_taskManager, &TaskManager::taskInfo, _mainWindow,
            &MainWindow::closeWithTaskInfo, QUEUED);
    connect(_mainWindow, &MainWindow::jobAdded, &_taskManager,
            &TaskManager::addJob, QUEUED);
    connect(_mainWindow, &MainWindow::getKeyId, &_taskManager,
            &TaskManager::getKeyId, QUEUED);
    connect(&_taskManager, &TaskManager::keyId, _mainWindow,
            &MainWindow::saveKeyId, QUEUED);
    connect(&_taskManager, &TaskManager::message, _mainWindow,
            &MainWindow::updateStatusMessage, QUEUED);
    connect(&_taskManager, &TaskManager::error, _mainWindow,
            &MainWindow::tarsnapError, QUEUED);
    connect(&_notification, &Notification::activated, _mainWindow,
            &MainWindow::notificationRaise, QUEUED);
    connect(&_notification, &Notification::messageClicked, _mainWindow,
            &MainWindow::notificationRaise, QUEUED);
    connect(&_journal, &Journal::journal, _mainWindow, &MainWindow::setJournal,
            QUEUED);
    connect(&_journal, &Journal::logEntry, _mainWindow,
            &MainWindow::appendToJournalLog, QUEUED);
    connect(_mainWindow, &MainWindow::clearJournal, &_journal, &Journal::purge,
            QUEUED);
    connect(_mainWindow, &MainWindow::findMatchingArchives, &_taskManager,
            &TaskManager::findMatchingArchives, QUEUED);
    connect(&_taskManager, &TaskManager::matchingArchives, _mainWindow,
            &MainWindow::matchingArchives, QUEUED);

    QMetaObject::invokeMethod(_mainWindow, "initializeMainWindow", QUEUED);
    QMetaObject::invokeMethod(&_taskManager, "loadArchives", QUEUED);
    QMetaObject::invokeMethod(&_taskManager, "loadJobs", QUEUED);
    QMetaObject::invokeMethod(&_journal, "getJournal", QUEUED);

    _mainWindow->show();
}

void AppGui::reinit()
{
    disconnect(&_taskManager, &TaskManager::displayNotification, &_notification,
               &Notification::displayNotification);
    disconnect(&_taskManager, &TaskManager::message, &_journal, &Journal::log);

    if(_mainWindow)
    {
        delete _mainWindow;
        _mainWindow = nullptr;
    }

    // reset existing persistent store and app settings
    PersistentStore &store = PersistentStore::instance();
    store.purge();

    QSettings settings;
    settings.setDefaultFormat(QSettings::NativeFormat);
    QSettings defaultSettings;
    if(defaultSettings.contains("app/wizard_done"))
    {
        defaultSettings.clear();
        defaultSettings.sync();
    }

    if(!initializeCore())
    {
        QMessageBox::critical(nullptr, tr("Failed to launch application"),
                              tr("An unknown error occurred."));
        exit(EXIT_FAILURE);
    }
    showMainWindow();
}

bool AppGui::runSetupWizard()
{
    // Show the first time setup dialog
    SetupDialog wizard;
    connect(&wizard, &SetupDialog::getTarsnapVersion, &_taskManager,
            &TaskManager::getTarsnapVersion);
    connect(&_taskManager, &TaskManager::tarsnapVersion, &wizard,
            &SetupDialog::setTarsnapVersion);
    connect(&wizard, &SetupDialog::requestRegisterMachine, &_taskManager,
            &TaskManager::registerMachine);
    connect(&_taskManager, &TaskManager::registerMachineStatus, &wizard,
            &SetupDialog::registerMachineStatus);
    connect(&wizard, &SetupDialog::initializeCache, &_taskManager,
            &TaskManager::initializeCache);
    connect(&_taskManager, &TaskManager::idle, &wizard,
            &SetupDialog::updateLoadingAnimation);

    if(QDialog::Rejected == wizard.exec())
    {
        quit();       // if we're running in the loop
        return false; // if called from main
    }
    return true;
}

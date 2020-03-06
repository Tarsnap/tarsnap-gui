#include "app-gui.h"

WARNINGS_DISABLE
#include <QFontDatabase>
#include <QMessageBox>
WARNINGS_ENABLE

#include <assert.h>

#include "debug.h"
#include "init-shared.h"
#include "translator.h"
#include "utils.h"

#include <ConsoleLog.h>
#include <TSettings.h>

AppGui::AppGui(int &argc, char **argv, struct optparse *opt)
    : QApplication(argc, argv),
      _mainWindow(nullptr),
      _journal(nullptr),
      _notification()
{
    // Sanity checks
    assert(opt != nullptr);
    assert(opt->check == 0);

    // Get values from optparse.  The (x == 1) is probably unnecessary, but
    // better safe than sorry!
    _jobsOption = (opt->jobs == 1);
    _configDir  = opt->config_dir;

    setQuitOnLastWindowClosed(false);

    qRegisterMetaType<QSystemTrayIcon::ActivationReason>(
        "QSystemTrayIcon::ActivationReason");

    // Set up the translator.
    TSettings settings;
    Translator::initializeTranslator();
    Translator &translator = Translator::instance();
    translator.translateApp(
        this, settings.value("app/language", LANG_AUTO).toString());
}

AppGui::~AppGui()
{
    if(_mainWindow)
        delete _mainWindow;
    _managerThread.quit();
    _managerThread.wait();
    if(_journal)
        delete _journal;
    Translator::destroy();
}

bool AppGui::handle_init(const QList<struct init_info> steps)
{
    // Handle each step of the initialization
    for(const struct init_info &info : steps)
    {
        if(!handle_step(info))
            return false;

        // There's no point trying to automatically process jobs with --dry-run.
        if((info.status == INIT_DRY_RUN) && _jobsOption)
            return false;

        // Special console output for "command-line-esque" --jobs option.  We
        // don't want it to be in AppCmdline, because we pop up a Notification
        // upon successful backup.
        // TODO: do we actually need/want those notifications?  What's the
        // balance between "ssh in and run `tarsnap-gui --jobs`" (i.e.
        // AppCmdline) vs. "put it on crontab, but allow a system Notification
        // window popup" (i.e. AppGui).
        if(_jobsOption)
        {
            if(info.status == INIT_SCHEDULE_OK)
                DEBUG << info.extra;
            else if(info.status == INIT_SCHEDULE_ERROR)
            {
                DEBUG << info.message;
                return false;
            }
        }
    }

    // We've finished initialization and can proceed to prepEventLoop().
    return true;
}

/*
 * Show message(s) (if applicable), and return false if there's an error.
 *
 * In order for Qt's translation tr() to work, this must be a class method
 * (rather than a static function).
 */
bool AppGui::handle_step(const struct init_info info)
{
    switch(info.status)
    {
    case INIT_OK:
        return true;
    case INIT_NEEDS_SETUP:
        QMessageBox::warning(nullptr, tr("Tarsnap warning"),
                             tr("Cannot complete setup."));
        return false;
    case INIT_DB_FAILED:
        QMessageBox::warning(nullptr, tr("Tarsnap warning"),
                             tr("Cannot initialize the database."));
        return false;
    case INIT_SETTINGS_RENAMED:
        QMessageBox::information(nullptr, tr("Tarsnap info"), info.message);
        return true;
    case INIT_DRY_RUN:
        QMessageBox::warning(nullptr, tr("Tarsnap warning"), info.message);
        return true;
    case INIT_SCHEDULE_OK:
        QMessageBox::information(nullptr, tr("Updated OS X launchd path"),
                                 info.message);
        return true;
    case INIT_SCHEDULE_ERROR:
        QMessageBox::information(nullptr,
                                 tr("Failed to updated OS X launchd path"),
                                 info.message);
        return true;
    }

    // Should not happen
    DEBUG << "AppGui: unexpected info.status:" << info.status;
    return false;
}

bool AppGui::prepEventLoop()
{
    // Initialize the PersistentStore early
    PersistentStore::instance();
    _journal = new Journal();

    // Queue loading the journal when we have an event loop.
    QMetaObject::invokeMethod(_journal, "load", QUEUED);

    connect(&_taskManager, &TaskManager::displayNotification, &_notification,
            &Notification::displayNotification, QUEUED);
    connect(&_taskManager, &TaskManager::message, _journal,
            &Journal::logMessage, QUEUED);

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

    connect(_mainWindow, &MainWindow::tarsnapVersionRequested, &_taskManager,
            &TaskManager::tarsnapVersionFind, QUEUED);
    connect(&_taskManager, &TaskManager::tarsnapVersionFound, _mainWindow,
            &MainWindow::tarsnapVersionResponse, QUEUED);
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
    connect(&_taskManager, &TaskManager::numTasks, _mainWindow,
            &MainWindow::updateNumTasks, QUEUED);
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
    connect(&_notification, &Notification::notification_clicked, _mainWindow,
            &MainWindow::handle_notification_clicked, QUEUED);
    connect(_journal, &Journal::journal, _mainWindow, &MainWindow::setJournal,
            QUEUED);
    connect(_journal, &Journal::logEntry, _mainWindow,
            &MainWindow::appendToJournalLog, QUEUED);
    connect(_mainWindow, &MainWindow::clearJournal, _journal, &Journal::purge,
            QUEUED);
    connect(_mainWindow, &MainWindow::findMatchingArchives, &_taskManager,
            &TaskManager::findMatchingArchives, QUEUED);
    connect(&_taskManager, &TaskManager::matchingArchives, _mainWindow,
            &MainWindow::matchingArchives, QUEUED);

    QMetaObject::invokeMethod(_mainWindow, "initializeMainWindow", QUEUED);
    QMetaObject::invokeMethod(&_taskManager, "loadArchives", QUEUED);
    QMetaObject::invokeMethod(&_taskManager, "loadJobs", QUEUED);
    QMetaObject::invokeMethod(_journal, "getJournal", QUEUED);

    _mainWindow->show();
}

void AppGui::reinit()
{
    TSettings settings;
    if(settings.contains("app/wizard_done"))
    {
        settings.clear();
    }

    QApplication::exit(EXIT_SUCCESS);
}

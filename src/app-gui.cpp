#include "app-gui.h"

WARNINGS_DISABLE
#include <QFontDatabase>
#include <QMessageBox>
WARNINGS_ENABLE

#include <assert.h>

#include "ConsoleLog.h"
#include "TSettings.h"

#include "backuptask.h"
#include "basetask.h"
#include "debug.h"
#include "init-shared.h"
#include "notification.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "persistentmodel/journal.h"
#include "taskmanager.h"
#include "translator.h"
#include "widgets/mainwindow.h"

AppGui::AppGui(int &argc, char **argv, struct optparse *opt)
    : QApplication(argc, argv),
      _mainWindow(nullptr),
      _journal(nullptr),
      _notification(new Notification()),
      _taskManager(new TaskManager())
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
    global_translator->translateApp(
        this, settings.value("app/language", LANG_AUTO).toString());
}

AppGui::~AppGui()
{
    if(_mainWindow)
        delete _mainWindow;
    if(_journal)
        delete _journal;
    delete _taskManager;
    delete _notification;
    Translator::destroy();
}

bool AppGui::handle_init(const QList<struct init_info> &steps)
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
bool AppGui::handle_step(const struct init_info &info)
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
    _journal = new Journal();

    // Queue loading the journal when we have an event loop.
    QMetaObject::invokeMethod(_journal, "load", Qt::QueuedConnection);

    connect(_taskManager, &TaskManager::displayNotification, _notification,
            &Notification::displayNotification, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::message, _journal, &Journal::logMessage,
            Qt::QueuedConnection);

    if(_jobsOption)
    {
        setQuitLockEnabled(true);
        connect(_notification, &Notification::activated, this,
                &AppGui::showMainWindow, Qt::QueuedConnection);
        connect(_notification, &Notification::messageClicked, this,
                &AppGui::showMainWindow, Qt::QueuedConnection);
        QMetaObject::invokeMethod(_taskManager, "runScheduledJobs",
                                  Qt::QueuedConnection);
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
    disconnect(_notification, &Notification::activated, this,
               &AppGui::showMainWindow);
    disconnect(_notification, &Notification::messageClicked, this,
               &AppGui::showMainWindow);

    _mainWindow = new MainWindow();
    Q_ASSERT(_mainWindow != nullptr);

    connect(_mainWindow, &MainWindow::tarsnapVersionRequested, _taskManager,
            &TaskManager::tarsnapVersionFind, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::tarsnapVersionFound, _mainWindow,
            &MainWindow::tarsnapVersionResponse, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::backupNow, _taskManager,
            &TaskManager::backupNow, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::getArchives, _taskManager,
            &TaskManager::getArchives, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::archiveList, _mainWindow,
            &MainWindow::archiveList, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::archiveAdded, _mainWindow,
            &MainWindow::addArchive, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::deleteArchives, _taskManager,
            &TaskManager::deleteArchives, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::loadArchiveStats, _taskManager,
            &TaskManager::getArchiveStats, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::loadArchiveContents, _taskManager,
            &TaskManager::getArchiveContents, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::numTasks, _mainWindow,
            &MainWindow::updateNumTasks, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::getOverallStats, _taskManager,
            &TaskManager::getOverallStats, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::overallStats, _mainWindow,
            &MainWindow::overallStatsChanged, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::repairCache, _taskManager,
            &TaskManager::fsck, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::nukeArchives, _taskManager,
            &TaskManager::nuke, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::restoreArchive, _taskManager,
            &TaskManager::restoreArchive, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::runSetupWizard, this, &AppGui::reinit,
            Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::stopTasks, _taskManager,
            &TaskManager::stopTasks, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::jobList, _mainWindow,
            &MainWindow::jobList, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::deleteJob, _taskManager,
            &TaskManager::deleteJob, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::jobAdded, _taskManager,
            &TaskManager::addJob, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::getKeyId, _taskManager,
            &TaskManager::getKeyId, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::keyId, _mainWindow,
            &MainWindow::saveKeyId, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::message, _mainWindow,
            &MainWindow::updateStatusMessage, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::error, _mainWindow,
            &MainWindow::tarsnapError, Qt::QueuedConnection);
    connect(_notification, &Notification::activated, _mainWindow,
            &MainWindow::notificationRaise, Qt::QueuedConnection);
    connect(_notification, &Notification::notification_clicked, _mainWindow,
            &MainWindow::handle_notification_clicked, Qt::QueuedConnection);
    connect(_journal, &Journal::journal, _mainWindow, &MainWindow::setJournal,
            Qt::QueuedConnection);
    connect(_journal, &Journal::logEntry, _mainWindow,
            &MainWindow::appendToJournalLog, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::clearJournal, _journal, &Journal::purge,
            Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::findMatchingArchives, _taskManager,
            &TaskManager::findMatchingArchives, Qt::QueuedConnection);
    connect(_taskManager, &TaskManager::matchingArchives, _mainWindow,
            &MainWindow::matchingArchives, Qt::QueuedConnection);

    connect(_mainWindow, &MainWindow::taskRequested, _taskManager,
            &TaskManager::queueGuiTask, Qt::QueuedConnection);
    connect(_mainWindow, &MainWindow::cancelTaskRequested, _taskManager,
            &TaskManager::cancelGuiTask, Qt::QueuedConnection);

    connect(this, &AppGui::lastWindowClosed, this,
            &AppGui::quitAfterEventsFinish, Qt::QueuedConnection);

    QMetaObject::invokeMethod(_mainWindow, "initializeMainWindow",
                              Qt::QueuedConnection);
    QMetaObject::invokeMethod(_taskManager, "loadArchives",
                              Qt::QueuedConnection);
    QMetaObject::invokeMethod(_taskManager, "loadJobs", Qt::QueuedConnection);
    QMetaObject::invokeMethod(_journal, "getJournal", Qt::QueuedConnection);

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

void AppGui::quitAfterEventsFinish()
{
    qApp->setQuitLockEnabled(true);
    quit();
}

#include "coreapplication.h"
#include "debug.h"
#include "utils.h"
#include "widgets/setupdialog.h"

#include <QDialog>
#include <QFontDatabase>
#include <QMessageBox>

CoreApplication::CoreApplication(int &argc, char **argv)
    : QApplication(argc, argv),
      _mainWindow(NULL),
      _notification(),
      _jobsOption(false)
{
    qRegisterMetaType<TaskStatus>("TaskStatus");
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
    qRegisterMetaType<BackupTaskPtr>("BackupTaskPtr");
    qRegisterMetaType<QList<ArchivePtr>>("QList<ArchivePtr >");
    qRegisterMetaType<ArchivePtr>("ArchivePtr");
    qRegisterMetaType<ArchiveRestoreOptions>("ArchiveRestoreOptions");
    qRegisterMetaType<QSqlQuery>("QSqlQuery");
    qRegisterMetaType<JobPtr>("JobPtr");
    qRegisterMetaType<QMap<QString, JobPtr>>("QMap<QString, JobPtr>");
    qRegisterMetaType<QSystemTrayIcon::ActivationReason>("QSystemTrayIcon::ActivationReason");
    qRegisterMetaType<TarsnapError>("TarsnapError");

    QCoreApplication::setOrganizationName(QLatin1String("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(QLatin1String("tarsnap.com"));
    QCoreApplication::setApplicationName(QLatin1String("Tarsnap"));
    QCoreApplication::setApplicationVersion(APP_VERSION);
}

CoreApplication::~CoreApplication()
{
    if(_mainWindow)
        delete _mainWindow;
}

void CoreApplication::parseArgs()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String("Tarsnap GUI - Online backups for the truly lazy"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption jobsOption(QStringList() << "j" << "jobs",
                                  tr("Executes all jobs sequentially that have the \'Include in scheduled backups\' option checked."
                                     " The application runs headless and useful information is printed to standard out and error."));
    QCommandLineOption appDataOption(QStringList() << "a" << "appdata",
                                    tr("Use the specified app data directory."
                                       " Useful for multiple configurations on the same machine (INI format is implied)."),
                                    tr("directory"));
    parser.addOption(jobsOption);
    parser.addOption(appDataOption);
    parser.process(arguments());
    _jobsOption = parser.isSet(jobsOption);
    _appDataDir = parser.value(appDataOption);
}

bool CoreApplication::initialize()
{
    parseArgs();

    QSettings settings;

    if(!_appDataDir.isEmpty())
    {
        settings.setPath(QSettings::IniFormat, QSettings::UserScope, _appDataDir);
        settings.setDefaultFormat(QSettings::IniFormat);
    }

    bool wizardDone = settings.value("app/wizard_done", false).toBool();
    if(!wizardDone)
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
        connect(&_taskManager, &TaskManager::idle, &wizard,
                &SetupDialog::updateLoadingAnimation);

        if(QDialog::Rejected == wizard.exec())
        {
            quit();       // if we're running in the loop
            return false; // if called from main
        }
    }

    bool iniFormat = settings.value("app/ini_format", false).toBool();
    if(iniFormat)
    {
        QString appData = settings.value("app/app_data", APPDATA).toString();
        settings.setPath(QSettings::IniFormat, QSettings::UserScope, appData);
        settings.setDefaultFormat(QSettings::IniFormat);
    }

    connect(&_taskManager, &TaskManager::displayNotification,
            &_notification, &Notification::displayNotification, QUEUED);
    connect(&_taskManager, &TaskManager::message, &_journal, &Journal::log,
            QUEUED);

    QMetaObject::invokeMethod(&_taskManager, "loadSettings", QUEUED);
    QMetaObject::invokeMethod(&_journal, "load", QUEUED);

    if(_jobsOption)
    {
        connect(&_notification, &Notification::activated, this,
                &CoreApplication::showMainWindow, QUEUED);
        connect(&_notification, &Notification::messageClicked, this,
                &CoreApplication::showMainWindow, QUEUED);
        _taskManager.setHeadless(true);
        QMetaObject::invokeMethod(&_taskManager, "runScheduledJobs", QUEUED);
    }
    else
    {
        showMainWindow();
    }

    return true;
}

void CoreApplication::showMainWindow()
{
    if(_mainWindow != NULL)
        return;

    _taskManager.setHeadless(false);
    disconnect(&_notification, &Notification::activated, this,
               &CoreApplication::showMainWindow);
    disconnect(&_notification, &Notification::messageClicked, this,
               &CoreApplication::showMainWindow);

    _mainWindow = new MainWindow();
    Q_ASSERT(_mainWindow != NULL);

    connect(_mainWindow, &MainWindow::getTarsnapVersion, &_taskManager,
            &TaskManager::getTarsnapVersion, QUEUED);
    connect(&_taskManager, &TaskManager::tarsnapVersion, _mainWindow,
            &MainWindow::setTarsnapVersion, QUEUED);
    connect(_mainWindow, &MainWindow::backupNow, &_taskManager,
            &TaskManager::backupNow, QUEUED);
    connect(_mainWindow, &MainWindow::loadArchives, &_taskManager,
            &TaskManager::loadArchives, QUEUED);
    connect(&_taskManager, &TaskManager::archiveList, _mainWindow,
            &MainWindow::archiveList, QUEUED);
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
            &MainWindow::updateSettingsSummary, QUEUED);
    connect(_mainWindow, &MainWindow::repairCache, &_taskManager,
            &TaskManager::fsck, QUEUED);
    connect(_mainWindow, &MainWindow::settingsChanged, &_taskManager,
            &TaskManager::loadSettings, QUEUED);
    connect(_mainWindow, &MainWindow::purgeArchives, &_taskManager,
            &TaskManager::nuke, QUEUED);
    connect(_mainWindow, &MainWindow::restoreArchive, &_taskManager,
            &TaskManager::restoreArchive, QUEUED);
    connect(_mainWindow, &MainWindow::runSetupWizard, this,
            &CoreApplication::reinit, QUEUED);
    connect(_mainWindow, &MainWindow::stopTasks, &_taskManager,
            &TaskManager::stopTasks, QUEUED);
    connect(_mainWindow, &MainWindow::loadJobs, &_taskManager,
            &TaskManager::loadJobs, QUEUED);
    connect(&_taskManager, &TaskManager::jobsList, _mainWindow,
            &MainWindow::jobsList, QUEUED);
    connect(_mainWindow, &MainWindow::deleteJob, &_taskManager,
            &TaskManager::deleteJob, QUEUED);
    connect(_mainWindow, &MainWindow::getTaskInfo, &_taskManager,
            &TaskManager::getTaskInfo, QUEUED);
    connect(&_taskManager, &TaskManager::taskInfo, _mainWindow,
            &MainWindow::displayStopTasks, QUEUED);
    connect(_mainWindow, &MainWindow::jobAdded, &_taskManager,
            &TaskManager::addJob, QUEUED);
    connect(&_taskManager, &TaskManager::message, _mainWindow,
            &MainWindow::updateStatusMessage, QUEUED);
    connect(&_taskManager, &TaskManager::error, _mainWindow,
            &MainWindow::tarsnapError, QUEUED);
    connect(&_notification, &Notification::activated, _mainWindow,
            &MainWindow::notificationRaise, QUEUED);
    connect(&_notification, &Notification::messageClicked, _mainWindow,
            &MainWindow::notificationRaise, QUEUED);
    connect(_mainWindow, &MainWindow::displayNotification, &_notification,
            &Notification::displayNotification, QUEUED);
    connect(_mainWindow, &MainWindow::logMessage, &_journal, &Journal::log,
            QUEUED);
    connect(_mainWindow, &MainWindow::getJournal, &_journal,
            &Journal::getJournal, QUEUED);
    connect(&_journal, &Journal::journal, _mainWindow,
            &MainWindow::setJournal, QUEUED);
    connect(&_journal, &Journal::logEntry, _mainWindow,
            &MainWindow::appendToJournalLog, QUEUED);

    QMetaObject::invokeMethod(_mainWindow, "loadArchives", QUEUED);
    QMetaObject::invokeMethod(_mainWindow, "loadJobs", QUEUED);
    QMetaObject::invokeMethod(_mainWindow, "getJournal", QUEUED);
    _mainWindow->show();
}

bool CoreApplication::reinit()
{
    if(_mainWindow)
    {
        delete _mainWindow;
        _mainWindow = NULL;
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

    return initialize();
}

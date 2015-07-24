#include "coreapplication.h"
#include "widgets/setupdialog.h"
#include "debug.h"

#include <QMessageBox>
#include <QDialog>
#include <QFontDatabase>

#define SUCCESS 0
#define FAILURE 1

CoreApplication::CoreApplication(int &argc, char **argv):
    QApplication(argc, argv), _mainWindow(NULL), _jobsOption(false)
{
    qRegisterMetaType< TaskStatus >("TaskStatus");
    qRegisterMetaType< QList<QUrl> >("QList<QUrl>");
    qRegisterMetaType< BackupTaskPtr >("BackupTaskPtr");
    qRegisterMetaType< QList<ArchivePtr > >("QList<ArchivePtr >");
    qRegisterMetaType< ArchivePtr >("ArchivePtr");
    qRegisterMetaType< ArchiveRestoreOptions >("ArchiveRestoreOptions");
    qRegisterMetaType< QSqlQuery >("QSqlQuery");
    qRegisterMetaType< JobPtr >("JobPtr");
    qRegisterMetaType< QMap<QString, JobPtr> >("QMap<QString, JobPtr>");

    QCoreApplication::setOrganizationName(QLatin1String("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(QLatin1String("tarsnap.com"));
    QCoreApplication::setApplicationName(QLatin1String("Tarsnap"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.5"));
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
    parser.addOption(jobsOption);
    parser.process(*this);
    _jobsOption = parser.isSet(jobsOption);
}

int CoreApplication::initialize()
{
    parseArgs();

    QSettings settings;
    if(!settings.value("application/wizardDone", false).toBool())
    {
        // Show the first time setup dialog
        SetupDialog wizard;
        connect(&wizard, SIGNAL(registerMachine(QString,QString,QString,QString,QString,QString))
                , &_taskManager, SLOT(registerMachine(QString,QString,QString,QString,QString,QString)));
        connect(&_taskManager, SIGNAL(registerMachineStatus(TaskStatus,QString)) , &wizard
                ,SLOT(registerMachineStatus(TaskStatus, QString)));
        connect(&_taskManager, SIGNAL(idle(bool)), &wizard, SLOT(updateLoadingAnimation(bool)), Qt::QueuedConnection);

        if(QDialog::Rejected == wizard.exec())
        {
            quit(); // if we're running in the loop
            return FAILURE; // if called from main
        }
    }

    QMetaObject::invokeMethod(&_taskManager, "loadSettings", Qt::QueuedConnection);

    if(_jobsOption)
    {
        QMetaObject::invokeMethod(&_taskManager, "runScheduledJobs", Qt::QueuedConnection);
    }
    else
    {
        // Show the main window
        _mainWindow = new MainWindow();
        if(!_mainWindow)
        {
            DEBUG << tr("Can't instantiate the MainWidget. Quitting.");
            return FAILURE;
        }

        connect(_mainWindow, SIGNAL(backupNow(BackupTaskPtr)), &_taskManager
                , SLOT(backupNow(BackupTaskPtr)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(getArchiveList()), &_taskManager
                , SLOT(getArchiveList()), Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(archiveList(QList<ArchivePtr>))
                , _mainWindow, SIGNAL(archiveList(QList<ArchivePtr>)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(deleteArchives(QList<ArchivePtr>)), &_taskManager,
                SLOT(deleteArchives(QList<ArchivePtr>)), Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(archivesDeleted(QList<ArchivePtr>)), _mainWindow
                , SLOT(archivesDeleted(QList<ArchivePtr>)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(loadArchiveStats(ArchivePtr)), &_taskManager
                ,SLOT(getArchiveStats(ArchivePtr)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(loadArchiveContents(ArchivePtr)), &_taskManager
                ,SLOT(getArchiveContents(ArchivePtr)), Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(idle(bool)), _mainWindow
                ,SLOT(updateLoadingAnimation(bool)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(getOverallStats()), &_taskManager
                , SLOT(getOverallStats()), Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(overallStats(quint64,quint64,quint64,quint64,quint64,qreal,QString))
                , _mainWindow,SLOT(updateSettingsSummary(quint64,quint64,quint64,quint64,quint64,qreal,QString)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(repairCache()), &_taskManager
                , SLOT(fsck()), Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(fsckStatus(TaskStatus,QString)), _mainWindow
                ,SLOT(repairCacheStatus(TaskStatus,QString)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(settingsChanged()), &_taskManager
                ,SLOT(loadSettings()), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(purgeArchives()), &_taskManager
                ,SLOT(nuke()), Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(nukeStatus(TaskStatus,QString)), _mainWindow
                ,SLOT(purgeArchivesStatus(TaskStatus,QString)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(restoreArchive(ArchivePtr,ArchiveRestoreOptions))
                ,&_taskManager, SLOT(restoreArchive(ArchivePtr,ArchiveRestoreOptions))
                , Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(restoreArchiveStatus(ArchivePtr,TaskStatus,QString)), _mainWindow
                , SLOT(restoreArchiveStatus(ArchivePtr,TaskStatus,QString)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(runSetupWizard()), this, SLOT(reinit()), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(stopTasks()), &_taskManager, SLOT(stopTasks()), Qt::QueuedConnection);
        connect(&_taskManager, SIGNAL(jobsList(QMap<QString,JobPtr>))
                , _mainWindow, SIGNAL(jobsList(QMap<QString,JobPtr>)), Qt::QueuedConnection);
        connect(_mainWindow, SIGNAL(deleteJob(JobPtr,bool)), &_taskManager, SLOT(deleteJob(JobPtr,bool)), Qt::QueuedConnection);

        QMetaObject::invokeMethod(&_taskManager, "loadJobs", Qt::QueuedConnection);
        QMetaObject::invokeMethod(&_taskManager, "getArchiveList", Qt::QueuedConnection);
        QMetaObject::invokeMethod(_mainWindow, "updateStatusMessage", Qt::QueuedConnection, Q_ARG(QString, tr("Refreshing archives list...")));
        _mainWindow->show();
    }

    return SUCCESS;
}

bool CoreApplication::reinit()
{
    if(_mainWindow)
    {
        delete _mainWindow;
        _mainWindow = 0;
    }

    // reset existing persistent store and app settings
    PersistentStore &store = PersistentStore::instance();
    store.purge();

    QSettings settings;
    if(settings.contains("application/wizardDone"))
    {
        settings.clear();
        settings.sync();
    }

    return initialize();
}


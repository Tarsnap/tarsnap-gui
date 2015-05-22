#include "coreapplication.h"
#include "widgets/setupdialog.h"
#include "debug.h"

#include <QMessageBox>
#include <QDialog>
#include <QFontDatabase>

#define SUCCESS 0
#define FAILURE 1

CoreApplication::CoreApplication(int &argc, char **argv):
    QApplication(argc, argv), _mainWindow(NULL)
{
    qRegisterMetaType< TaskStatus >("TaskStatus");
    qRegisterMetaType< QList<QUrl> >("QList<QUrl>");
    qRegisterMetaType< BackupTaskPtr >("BackupTaskPtr");
    qRegisterMetaType< QList<ArchivePtr > >("QList<ArchivePtr >");
    qRegisterMetaType< ArchivePtr >("ArchivePtr");
    qRegisterMetaType< ArchiveRestoreOptions >("ArchiveRestoreOptions");
    qRegisterMetaType< QSqlQuery >("QSqlQuery");
    qRegisterMetaType< JobPtr >("JobPtr");
}

CoreApplication::~CoreApplication()
{
    if(_mainWindow)
        delete _mainWindow;
}

int CoreApplication::initialize()
{
    QCoreApplication::setOrganizationName(tr("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(tr("tarsnap.com"));
    QCoreApplication::setApplicationName(tr("Tarsnap"));

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
    connect(&_taskManager, SIGNAL(overallStats(qint64,qint64,qint64,qint64,qint64,qreal,QString))
            , _mainWindow,SLOT(updateSettingsSummary(qint64,qint64,qint64,qint64,qint64,qreal,QString)), Qt::QueuedConnection);
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

    QMetaObject::invokeMethod(&_taskManager, "getArchiveList", Qt::QueuedConnection);
    _mainWindow->show();

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
    if(store.initialized())
        store.purge();

    QSettings settings;
    if(settings.contains("application/wizardDone"))
    {
        settings.clear();
        settings.sync();
    }

    return initialize();
}


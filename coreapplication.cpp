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
    QCoreApplication::setApplicationName(tr("Tarsnappy"));

    QSettings settings;
    if(!settings.value("application/wizardDone", false).toBool())
    {
        // Show the first time setup dialog
        if(false == runSetupWizard())
            return FAILURE;
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
    connect(&_taskManager, SIGNAL(backupTaskUpdate(BackupTaskPtr))
            , _mainWindow, SLOT(backupTaskUpdate(BackupTaskPtr)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(getArchivesList()), &_taskManager
            , SLOT(getArchivesList()), Qt::QueuedConnection);
    connect(&_taskManager, SIGNAL(archivesList(QList<ArchivePtr>))
            , _mainWindow, SIGNAL(archivesList(QList<ArchivePtr>)), Qt::QueuedConnection);
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
            , SLOT(runFsck()), Qt::QueuedConnection);
    connect(&_taskManager, SIGNAL(fsckStatus(TaskStatus,QString)), _mainWindow
            ,SLOT(repairCacheStatus(TaskStatus,QString)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(settingsChanged()), &_taskManager
            ,SLOT(loadSettings()), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(settingsChanged()), _mainWindow
            ,SLOT(loadSettings()), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(purgeArchives()), &_taskManager
            ,SLOT(nukeArchives()), Qt::QueuedConnection);
    connect(&_taskManager, SIGNAL(nukeStatus(TaskStatus,QString)), _mainWindow
            ,SLOT(purgeArchivesStatus(TaskStatus,QString)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(restoreArchive(ArchivePtr,ArchiveRestoreOptions))
            ,&_taskManager, SLOT(restoreArchive(ArchivePtr,ArchiveRestoreOptions))
            , Qt::QueuedConnection);
    connect(&_taskManager, SIGNAL(restoreArchiveStatus(ArchivePtr,TaskStatus,QString)), _mainWindow
            , SLOT(restoreArchiveStatus(ArchivePtr,TaskStatus,QString)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(runSetupWizard()), this, SLOT(runSetupWizard()), Qt::QueuedConnection);

    QMetaObject::invokeMethod(&_taskManager, "getArchivesList", Qt::QueuedConnection);
    _mainWindow->show();

    return SUCCESS;
}

bool CoreApplication::runSetupWizard()
{
    SetupDialog wizard;
    connect(&wizard, SIGNAL(registerMachine(QString,QString,QString,QString,QString,QString))
            , &_taskManager, SLOT(registerMachine(QString,QString,QString,QString,QString,QString)));
    connect(&_taskManager, SIGNAL(registerMachineStatus(TaskStatus,QString))
            , &wizard, SLOT(registerMachineStatus(TaskStatus, QString)));
    connect(&_taskManager, SIGNAL(idle(bool)), &wizard
            , SLOT(updateLoadingAnimation(bool)), Qt::QueuedConnection);
    if(QDialog::Rejected == wizard.exec())
    {
        return false;
    }
    else
    {
        if(_mainWindow)
            QMetaObject::invokeMethod(_mainWindow, "settingsChanged", Qt::QueuedConnection);
    }
    return true;
}


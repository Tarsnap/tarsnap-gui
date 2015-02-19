#include "coreapplication.h"
#include "setupdialog.h"
#include "debug.h"

#include <QMessageBox>
#include <QDialog>
#include <QFontDatabase>

#define SUCCESS 0
#define FAILURE 1

CoreApplication::CoreApplication(int &argc, char **argv):
    QApplication(argc, argv), _mainWindow(NULL)
{
    qRegisterMetaType<JobStatus>("JobStatus");
    qRegisterMetaType< QList<QUrl> >("QList<QUrl>");
    qRegisterMetaType<BackupJob>("BackupJob");
    qRegisterMetaType< BackupJobPtr >("BackupJobPtr");
    qRegisterMetaType< QList<ArchivePtr> >("QList<ArchivePtr >");
    qRegisterMetaType< ArchivePtr >("ArchivePtr");

    QCoreApplication::setOrganizationName(tr("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(tr("tarsnap.com"));
    QCoreApplication::setApplicationName(tr("Tarsnappy"));

#ifdef Q_OS_FREEBSD
    QFont font;
    font.setFamily("Helvetica");
    font.setHintingPreference(QFont::PreferFullHinting);
    font.setStyleHint(QFont::Helvetica);
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setPointSize(9);
    setFont(font);
#endif

    QSettings settings;
    if(!settings.value("application/wizardDone", false).toBool())
    {
        // Show the first time setup dialog
        runSetupWizard();
    }

    QMetaObject::invokeMethod(&_jobManager, "loadSettings", Qt::QueuedConnection);

    // Show the main window
    _mainWindow = new MainWindow();
    if(!_mainWindow)
    {
        DEBUG << tr("Can't instantiate the MainWidget. Quitting.");
        quitApplication(FAILURE);
    }

    connect(_mainWindow, SIGNAL(backupNow(BackupJobPtr)), &_jobManager
            , SLOT(backupNow(BackupJobPtr)), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(backupJobUpdate(BackupJobPtr))
            , _mainWindow, SLOT(backupJobUpdate(BackupJobPtr)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(getArchivesList()), &_jobManager
            , SLOT(getArchivesList()), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(archivesList(QList<ArchivePtr>))
            , _mainWindow, SIGNAL(archivesList(QList<ArchivePtr>)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(deleteArchives(QList<ArchivePtr>)), &_jobManager,
            SLOT(deleteArchives(QList<ArchivePtr>)), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(archivesDeleted(QList<ArchivePtr>)), _mainWindow
            , SLOT(archivesDeleted(QList<ArchivePtr>)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(loadArchiveStats(ArchivePtr)), &_jobManager
            ,SLOT(getArchiveStats(ArchivePtr)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(loadArchiveContents(ArchivePtr)), &_jobManager
            ,SLOT(getArchiveContents(ArchivePtr)), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(idle(bool)), _mainWindow
            ,SLOT(updateLoadingAnimation(bool)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(getOverallStats()), &_jobManager
            , SLOT(getOverallStats()), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(overallStats(qint64,qint64,qint64,qint64,qint64,qreal,QString))
            , _mainWindow,SLOT(updateSettingsSummary(qint64,qint64,qint64,qint64,qint64,qreal,QString)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(repairCache()), &_jobManager
            , SLOT(runFsck()), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(fsckStatus(JobStatus,QString)), _mainWindow
            ,SLOT(repairCacheStatus(JobStatus,QString)), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(settingsChanged()), &_jobManager
            ,SLOT(loadSettings()), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(settingsChanged()), _mainWindow
            ,SLOT(loadSettings()), Qt::QueuedConnection);
    connect(_mainWindow, SIGNAL(purgeArchives()), &_jobManager
            ,SLOT(nukeArchives()), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(nukeStatus(JobStatus,QString)), _mainWindow
            ,SLOT(purgeArchivesStatus(JobStatus,QString)), Qt::QueuedConnection);

    QMetaObject::invokeMethod(&_jobManager, "getArchivesList", Qt::QueuedConnection);

    connect(_mainWindow, SIGNAL(runSetupWizard()), this, SLOT(runSetupWizard()), Qt::QueuedConnection);
    _mainWindow->show();
}

CoreApplication::~CoreApplication()
{
    if(_mainWindow)
        delete _mainWindow;
}

void CoreApplication::runSetupWizard()
{
    QSettings settings;
    SetupDialog wizard;
    connect(&wizard, SIGNAL(registerMachine(QString,QString,QString,QString,QString,QString))
            , &_jobManager, SLOT(registerMachine(QString,QString,QString,QString,QString,QString)));
    connect(&_jobManager, SIGNAL(registerMachineStatus(JobStatus,QString))
            , &wizard, SLOT(registerMachineStatus(JobStatus, QString)));
    connect(&_jobManager, SIGNAL(idle(bool)), &wizard
            , SLOT(updateLoadingAnimation(bool)), Qt::QueuedConnection);
    bool wizardDone = true;
    int returnCode = wizard.exec();
    if(returnCode == QDialog::Rejected)
    {
        QMessageBox::StandardButton confirm = QMessageBox::question(&wizard, tr("Confirm action")
                                                                    ,tr("Display the wizard next time when Tarsnap is started?")
                                                                    ,( QMessageBox::Yes | QMessageBox::No ), QMessageBox::Yes);
        if(confirm == QMessageBox::Yes)
            wizardDone = false;
    }
    if(wizardDone)
    {
        settings.setValue("application/wizardDone", true);
        settings.sync();
    }
    if(_mainWindow)
        QMetaObject::invokeMethod(_mainWindow, "settingsChanged", Qt::QueuedConnection);
}

void CoreApplication::quitApplication(int returnCode)
{
    exit(returnCode);
}


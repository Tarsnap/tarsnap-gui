#include "coreapplication.h"
#include "setupdialog.h"

#include <QDebug>

#define SUCCESS 0
#define FAILURE 1

CoreApplication::CoreApplication(int &argc, char **argv):
    QApplication(argc, argv)
{
    qSetMessagePattern("%{file}(%{line}): %{message}");

    qRegisterMetaType<JobStatus>("JobStatus");
    qRegisterMetaType< QList<QUrl> >("QList<QUrl>");
    qRegisterMetaType<BackupJob>("BackupJob");
    qRegisterMetaType< QSharedPointer<BackupJob> >("QSharedPointer<BackupJob>");

    QCoreApplication::setOrganizationName(tr("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(tr("tarsnap.com"));
    QCoreApplication::setApplicationName(tr("Tarsnappy"));

    QSettings settings;
    if(!settings.value("application/wizardDone", false).toBool())
    {
        // Show the first time setup dialog
        SetupDialog wizard;
        connect(&wizard, SIGNAL(registerMachine(QString,QString,QString,QString))
                ,&_jobManager, SLOT(registerMachine(QString,QString,QString,QString)));
        connect(&_jobManager, SIGNAL(registerMachineStatus(JobStatus,QString))
                , &wizard, SLOT(registerMachineStatus(JobStatus, QString)));
        wizard.exec();
        settings.setValue("application/wizardDone", true);
        settings.sync();
    }

    // Show the main window
    _mainWindow = new MainWindow();
    if(!_mainWindow)
    {
        qDebug() << tr("Can't instantiate the MainWidget. Quitting.");
        quitApplication(FAILURE);
    }

    connect(_mainWindow, SIGNAL(backupNow(QSharedPointer<BackupJob>)), &_jobManager
            , SLOT(backupNow(QSharedPointer<BackupJob>)), Qt::QueuedConnection);
    connect(&_jobManager, SIGNAL(jobUpdate(QSharedPointer<BackupJob>))
            , _mainWindow, SLOT(jobUpdate(QSharedPointer<BackupJob>)));

    _mainWindow->show();
}

CoreApplication::~CoreApplication()
{
    if(_mainWindow)
        delete _mainWindow;
}

void CoreApplication::quitApplication(int returnCode)
{
    exit(returnCode);
}


#include "coreapplication.h"
#include "setupdialog.h"

#include <QDebug>

#define SUCCESS 0
#define FAILURE 1

CoreApplication::CoreApplication(int &argc, char **argv):
    QApplication(argc, argv)
{
    qSetMessagePattern("%{file}(%{line}): %{message}");

    qRegisterMetaType<JobManager::JobStatus>("JobManager::JobStatus");

    QCoreApplication::setOrganizationName(tr("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(tr("tarsnap.com"));
    QCoreApplication::setApplicationName(tr("Tarsnappy"));

    QSettings settings;
    if(!settings.value("application/wizardComplete", false).toBool())
    {
        // Show the first time setup dialog
        SetupDialog *dialog = new SetupDialog();
        connect(dialog, SIGNAL(finished(int)), dialog, SLOT(setupDialogDone(int)));
        connect(dialog, SIGNAL(registerMachine(QString,QString,QString,QString))
                ,&_jobManager, SLOT(registerMachine(QString,QString,QString,QString)));
        connect(&_jobManager, SIGNAL(registerMachineStatus(JobManager::JobStatus,QString))
                , dialog, SLOT(registerMachineStatus(JobManager::JobStatus, QString)));
        dialog->exec();
        delete dialog;
    }

    // Show the main window
    _mainWindow = new MainWindow();
    if(!_mainWindow)
    {
        qDebug() << tr("Can't instantiate the MainWidget. Quitting.");
        quitApplication(FAILURE);
    }
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

void CoreApplication::setupWizardDone(int result)
{
    Q_UNUSED(result)

    QSettings settings;
    settings.setValue("application/wizardComplete", true);
}

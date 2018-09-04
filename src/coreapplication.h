#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include "notification.h"
#include "persistentmodel/journal.h"
#include "taskmanager.h"
#include "widgets/mainwindow.h"

extern "C" {
#include "optparse.h"
}

#include <QApplication>
#include <QObject>
#include <QSettings>

/*!
 * \ingroup misc
 * \brief the CoreApplication is a QApplication which launches the MainWindow,
 * SetupWindow (if applicable), and manages background tasks.
 */
class CoreApplication : public QApplication
{
    Q_OBJECT

public:
    //! Constructor.
    CoreApplication(int &argc, char **argv, struct optparse *opt);
    ~CoreApplication();

    //! Initializes the QSettings, Translator, PersistentStore, and launches
    //! the Setup wizard (if necessary).
    bool initializeCore();

private slots:
    //! Removes the MainWindow (if it exists), resets the PersistentStore
    //! and app Settings, then runs initializeCore().  Used as a preliminary
    //! step before the SetupWizard.
    bool reinit();
    //! Creates a MainWindow.
    void showMainWindow();

private:
    MainWindow * _mainWindow;
    Notification _notification;
    TaskManager  _taskManager;
    QThread      _managerThread;
    Journal      _journal;
    bool         _jobsOption;
    QString      _appDataDir;
    bool         _checkOption;
};

#endif // COREAPPLICATION_H

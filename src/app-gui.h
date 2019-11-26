#ifndef APP_GUI_H
#define APP_GUI_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QApplication>
#include <QObject>
WARNINGS_ENABLE

#include "notification.h"
#include "persistentmodel/journal.h"
#include "taskmanager.h"
#include "widgets/mainwindow.h"

extern "C" {
#include "optparse.h"
}

/*!
 * \ingroup misc
 * \brief The AppGui is a QApplication which launches the MainWindow,
 * SetupDialog (if applicable), and manages background tasks.
 */
class AppGui : public QApplication
{
    Q_OBJECT

public:
    //! Constructor.
    AppGui(int &argc, char **argv, struct optparse *opt);
    ~AppGui();

    //! Initializes the Settings, Translator, PersistentStore, and launches
    //! the Setup wizard (if necessary).
    //! \return True if the initialization was successful.
    bool initializeCore();

    //! Prepare to pass control to the QEventLoop.
    //! \return True if the calling function should call app.exec().
    bool prepMainLoop();

private slots:
    //! Removes the MainWindow (if it exists), resets the PersistentStore
    //! and app Settings, then runs initializeCore().  Used as a preliminary
    //! step before the SetupWizard.
    void reinit();
    //! Creates a MainWindow.
    void showMainWindow();

private:
    MainWindow * _mainWindow;
    Notification _notification;
    TaskManager  _taskManager;
    QThread      _managerThread;
    Journal      _journal;
    bool         _jobsOption;
    QString      _configDir;
    bool         _checkOption;

    bool runSetupWizard();
};

#endif // APP_GUI_H

#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include "notification.h"
#include "taskmanager.h"
#include "persistentmodel/journal.h"
#include "widgets/mainwindow.h"

#include <QApplication>
#include <QObject>
#include <QSettings>

class CoreApplication : public QApplication
{
    Q_OBJECT

public:
    CoreApplication(int &argc, char **argv);
    ~CoreApplication();

    void parseArgs();
    bool initialize();

public slots:
    bool reinit();
    void showMainWindow();

private:
    MainWindow  *_mainWindow;
    Notification _notification;
    TaskManager  _taskManager;
    QThread      _managerThread;
    Journal      _journal;
    bool         _jobsOption;
    QString      _appDataDir;
};

#endif // COREAPPLICATION_H

#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include "widgets/mainwindow.h"
#include "taskmanager.h"
#include "notification.h"

#include <QObject>
#include <QApplication>
#include <QSettings>

class CoreApplication : public QApplication
{
    Q_OBJECT

public:
    CoreApplication(int &argc, char **argv);
    ~CoreApplication();

    void parseArgs();
    bool initialize();
    void showMainWindow();

public slots:
    bool reinit();

private:
    MainWindow  *_mainWindow;
    Notification _notification;
    TaskManager  _taskManager;
    bool         _jobsOption;
    QString      _appDataDir;
};

#endif // COREAPPLICATION_H

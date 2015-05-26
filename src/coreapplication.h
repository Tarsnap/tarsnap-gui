#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include "widgets/mainwindow.h"
#include "taskmanager.h"

#include <QObject>
#include <QApplication>
#include <QSettings>

class CoreApplication : public QApplication
{
    Q_OBJECT

public:
    CoreApplication(int &argc, char **argv);
    ~CoreApplication();

    int initialize();

public slots:
    bool reinit();

private:
    MainWindow  *_mainWindow;
    TaskManager  _taskManager;
};

#endif // COREAPPLICATION_H

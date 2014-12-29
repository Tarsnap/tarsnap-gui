#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H

#include "mainwidget.h"

#include <QObject>
#include <QApplication>
#include <QSettings>

class CoreApplication : public QApplication
{
    Q_OBJECT

public:
    CoreApplication(int &argc, char **argv);
    ~CoreApplication();

    void quitApplication(int returnCode);

private:
    MainWidget *_mainWindow;
    QSettings   _settings;
};

#endif // COREAPPLICATION_H

#ifndef TEST_APP_H
#define TEST_APP_H

#include <QCoreApplication>
#include <QEventLoop>

class App : public QCoreApplication
{
    Q_OBJECT

public:
    App(int &argc, char **argv) : QCoreApplication(argc, argv) {}

    void process()
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
    }
};

#endif

#ifndef TEST_APP_H
#define TEST_APP_H

#include <QCoreApplication>

class App : public QCoreApplication
{
    Q_OBJECT

public:
    App(int &argc, char **argv) : QCoreApplication(argc, argv) {}

    void process() { QCoreApplication::processEvents(0, 1000); }
};

#endif

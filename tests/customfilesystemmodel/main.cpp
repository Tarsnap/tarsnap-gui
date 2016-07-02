#include <QApplication>
#include "testCFSM.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestCFSM tester;
    return app.exec();
}

#include "coreapplication.h"
#include "debug.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
    ConsoleLog::instance().initializeConsoleLog();
    CoreApplication app(argc, argv);
    if(app.initializeCore())
        return app.exec();
    else
        return EXIT_SUCCESS;
}

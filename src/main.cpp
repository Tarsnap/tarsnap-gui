#include "coreapplication.h"
#include "debug.h"

#include <stdlib.h>

int main(int argc, char *argv[])
{
    ConsoleLog::instance().initialize();
    CoreApplication app(argc, argv);
    if(app.initialize())
        return app.exec();
    else
        return EXIT_SUCCESS;
}

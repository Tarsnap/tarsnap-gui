#include "coreapplication.h"
#include "debug.h"

extern "C" {
#include "optparse.h"
#include "warnp.h"
}

#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct optparse *opt;

    // Initialize debug messages.
    WARNP_INIT;
    ConsoleLog::instance().initializeConsoleLog();

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == NULL)
        exit(1);

    // Basic initialization that cannot fail.
    CoreApplication app(argc, argv, opt);
    optparse_free(opt);

    // Run more complicated initialization.
    if(!app.initializeCore())
        return EXIT_FAILURE;

    // If we want the GUI or have any tasks, do them.
    if(app.prepMainLoop())
        return (app.exec());
    else
        return EXIT_SUCCESS;
}

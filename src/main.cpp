#include "app-cmdline.h"
#include "debug.h"

#ifdef QT_GUI_LIB
#include "app-gui.h"
#endif

extern "C" {
#include "optparse.h"
#include "warnp.h"
}

#include <TSettings.h>

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

    // Should we use the gui or non-gui app?
    if(opt->check == 0)
    {
#ifdef QT_GUI_LIB
        // Basic initialization that cannot fail.
        AppGui app(argc, argv, opt);
        optparse_free(opt);

        // Run more complicated initialization.
        if(!app.initializeCore())
            return EXIT_FAILURE;

        // If we want the GUI or have any tasks, do them.
        if(app.prepMainLoop())
            return (app.exec());
        else
            return EXIT_SUCCESS;
#else
        qDebug() << "This binary does not support GUI operations.  Try:\n\t"
                 << argv[0] << "-h";
        exit(1);
#endif
    }
    else
    {
        // Basic initialization that cannot fail.
        AppCmdline app(argc, argv, opt);
        optparse_free(opt);

        // Run more complicated initialization.
        if(!app.initializeCore())
            return EXIT_FAILURE;

        // If we want have any tasks, do them.
        if(app.prepMainLoop())
            return (app.exec());
        else
            return EXIT_SUCCESS;
    }

    // Clean up
    TSettings::destroy();
}

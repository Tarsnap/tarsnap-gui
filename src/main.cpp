#include "app-cmdline.h"

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
    int              ret;

    // Initialize debug messages.
    WARNP_INIT;

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == NULL)
    {
        ret = EXIT_FAILURE;
        goto done;
    }

    // Should we use the gui or non-gui app?
    if(opt->check == 0)
    {
#ifdef QT_GUI_LIB
        // Basic initialization that cannot fail.
        AppGui app(argc, argv, opt);

        // Run more complicated initialization.
        if(!app.initializeCore())
        {
            ret = EXIT_FAILURE;
            goto done;
        }

        // If we want the GUI or have any tasks, do them.
        if(app.prepMainLoop())
            ret = app.exec();
        else
            ret = EXIT_SUCCESS;
#else
        warn0("This binary does not support GUI operations");
        ret = 1;
        goto done;
#endif
    }
    else
    {
        // Basic initialization that cannot fail.
        AppCmdline app(argc, argv, opt);

        // Run more complicated initialization.
        if(!app.initializeCore())
        {
            ret = EXIT_FAILURE;
            goto done;
        }

        // If we want have any tasks, do them.
        if(app.prepMainLoop())
            ret = app.exec();
        else
            ret = EXIT_SUCCESS;
    }

done:
    // Clean up
    TSettings::destroy();
    optparse_free(opt);

    return (ret);
}

#include <stdlib.h>

#include "warnings-disable.h"

extern "C" {
#include "optparse.h"
WARNINGS_DISABLE
#include "warnp.h"
WARNINGS_ENABLE
}

#include <TSettings.h>

#include "app-cmdline.h"
#ifdef QT_GUI_LIB
#include "app-gui.h"
#endif
#include "init-shared.h"

static int run_cmdline(int argc, char *argv[], struct optparse *opt)
{
    int ret;

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

done:
    init_shared_free();
    return (ret);
}

static int run_gui(int argc, char *argv[], struct optparse *opt)
{
    int ret;

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

done:
#else
    (void)argc;
    (void)argv;
    (void)opt;

    warn0("This binary does not support GUI operations");
    ret = 1;
#endif

    init_shared_free();
    return (ret);
}

int main(int argc, char *argv[])
{
    struct optparse *opt;
    int              ret;

    // Initialize debug messages.
    WARNP_INIT;

    // Parse command-line arguments
    if((opt = optparse_parse(argc, argv)) == nullptr)
    {
        ret = EXIT_FAILURE;
        goto done;
    }

    // Should we use the gui or non-gui app?
    if(opt->check == 0)
        ret = run_gui(argc, argv, opt);
    else
        ret = run_cmdline(argc, argv, opt);

done:
    optparse_free(opt);

    return (ret);
}

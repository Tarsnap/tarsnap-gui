#include <stdlib.h>

#include "warnings-disable.h"

extern "C" {
#include "optparse.h"
WARNINGS_DISABLE
#include "warnp.h"
WARNINGS_ENABLE
}

WARNINGS_DISABLE
#include <QList>
WARNINGS_ENABLE

#include "app-cmdline.h"
#ifdef QT_GUI_LIB
#include "app-gui.h"
#include "app-setup.h"
#endif
#include "init-shared.h"

static int run_cmdline(int argc, char *argv[], struct optparse *opt)
{
    int ret;

    // Initialization that doesn't require a QCoreApplication.
    const QList<struct init_info> steps = init_shared(opt->config_dir);

    // Basic initialization that cannot fail.
    AppCmdline app(argc, argv, opt);

    // Act on any initialization failures.
    if(!app.handle_init(steps))
    {
        ret = EXIT_FAILURE;
        goto done;
    }

    // Should we launch the event loop?
    if(app.prepEventLoop())
        ret = app.exec();
    else
        ret = EXIT_SUCCESS;

done:
    init_shared_free();
    return (ret);
}

static int run_gui_main(int argc, char *argv[], struct optparse *opt,
                        const QList<struct init_info> &steps)
{
    int ret;

#ifdef QT_GUI_LIB
    // Basic initialization that cannot fail.
    AppGui app(argc, argv, opt);

    // Act on any initialization failures.
    if(!app.handle_init(steps))
    {
        ret = EXIT_FAILURE;
        goto done;
    }

    // Should we launch the event loop?
    if(app.prepEventLoop())
        ret = app.exec();
    else
        ret = EXIT_SUCCESS;

done:
#else
    (void)argc;
    (void)argv;
    (void)opt;
    (void)steps;

    warn0("This binary does not support GUI operations");
    ret = 1;
#endif

    return (ret);
}

static int run_gui_setup(int argc, char *argv[],
                         const QList<struct init_info> &steps)
{
    int ret;

#ifdef QT_GUI_LIB
    // Basic initialization that cannot fail.
    AppSetup app(argc, argv);

    // Act on any initialization failures.
    if(!app.handle_init(steps))
    {
        ret = EXIT_FAILURE;
        goto done;
    }

    // Should we launch the event loop?
    if(app.prepEventLoop())
        ret = app.exec();
    else
        ret = EXIT_FAILURE;

done:
#else
    (void)argc;
    (void)argv;
    (void)steps;

    warn0("This binary does not support GUI operations");
    ret = 1;
#endif

    return (ret);
}

static int run_gui(int argc, char *argv[], struct optparse *opt)
{
    int ret;

    while(true)
    {
        // Initialization that doesn't require a QCoreApplication.
        const QList<struct init_info> steps = init_shared(opt->config_dir);

        // Initialize & launch setup (if applicable).
        if(init_shared_need_setup())
        {
            if((ret = run_gui_setup(argc, argv, steps)) != 0)
                goto done;
        }
        else
        {
            // Initialize & launch main GUI.
            if((ret = run_gui_main(argc, argv, opt, steps)) != 0)
                goto done;

            // If we don't need to re-run the setup wizard, exit without an
            // error.
            if(!init_shared_need_setup())
                goto done;
        }
    }

done:
    init_shared_free();

    return (ret);
}

int main(int argc, char *argv[])
{
    struct optparse *opt;
    int              ret;

    // Initialize debug messages.
    WARNP_INIT;

    // Parse command-line arguments.
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

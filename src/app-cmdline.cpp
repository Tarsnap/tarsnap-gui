#include "app-cmdline.h"
#include "debug.h"
#include "init-shared.h"
#include "scheduling.h"
#include "translator.h"
#include "utils.h"

#include <assert.h>

AppCmdline::AppCmdline(int &argc, char **argv, struct optparse *opt)
    : QCoreApplication(argc, argv)
{
    // Sanity check
    assert(opt != NULL);

    // Get values from optparse.  The (x == 1) is probably unnecessary, but
    // better safe than sorry!
    _checkOption = (opt->check == 1);
    _configDir   = opt->config_dir;

    init_shared(this);
}

AppCmdline::~AppCmdline()
{
    _managerThread.quit();
    _managerThread.wait();
}

bool AppCmdline::initializeCore()
{
    struct init_info info = init_shared_core(this, _configDir);

    switch(info.status)
    {
    case INIT_OK:
        break;
    case INIT_NEEDS_SETUP:
        DEBUG << "Cannot proceed without a config file.";
        return false;
    case INIT_DRY_RUN:
    case INIT_SCHEDULE_ERROR:
        DEBUG << info.message;
        // Don't return an error, because these aren't critical errors
        break;
    case INIT_SCHEDULE_OK:
        // info.message contains longer text intended for a GUI message box.
        DEBUG << info.extra;
        break;
    }

    // We don't have anything else to do
    return true;
}

bool AppCmdline::prepMainLoop()
{
    return false;
}

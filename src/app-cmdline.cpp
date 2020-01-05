#include "app-cmdline.h"

#include <assert.h>

#include "debug.h"
#include "init-shared.h"

#include <persistentmodel/persistentstore.h>
#include <translator.h>

#include <ConsoleLog.h>

AppCmdline::AppCmdline(int &argc, char **argv, struct optparse *opt)
    : QCoreApplication(argc, argv)
{
    // Sanity check
    assert(opt != nullptr);

    // Get values from optparse.  The (x == 1) is probably unnecessary, but
    // better safe than sorry!
    _checkOption = (opt->check == 1);
    _configDir   = opt->config_dir;

    init_shared(this);
}

AppCmdline::~AppCmdline()
{
    PersistentStore::destroy();
    Translator::destroy();
    ConsoleLog::destroy();
}

bool AppCmdline::initializeCore()
{
    struct init_info info;

    // Set up Settings.
    info = init_shared_settings(_configDir);
    if(info.status == INIT_SETTINGS_RENAMED)
        DEBUG << info.message;

    // Set up the Translator, check --dry-run, update scheduling path.
    info = init_shared_core(this);

    switch(info.status)
    {
    case INIT_OK:
        break;
    case INIT_NEEDS_SETUP:
        DEBUG << "Cannot proceed without a config file.";
        return false;
    case INIT_DB_FAILED:
        DEBUG << "Cannot initialize the database.";
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
    case INIT_SETTINGS_RENAMED:
        DEBUG << "Got INIT_SETTINGS_RENAMED; should not happen here!";
        return false;
    }

    // We don't have anything else to do
    return true;
}

bool AppCmdline::prepMainLoop()
{
    return false;
}

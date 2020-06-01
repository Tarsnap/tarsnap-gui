#include "app-cmdline.h"

WARNINGS_DISABLE
#include <QVariant>
WARNINGS_ENABLE

#include <assert.h>

extern "C" {
#include "optparse.h"
}

#include "TSettings.h"

#include "debug.h"
#include "init-shared.h"
#include "translator.h"

AppCmdline::AppCmdline(int &argc, char **argv, struct optparse *opt)
    : QCoreApplication(argc, argv)
{
    // Sanity check
    assert(opt != nullptr);

    // Get values from optparse.  The (x == 1) is probably unnecessary, but
    // better safe than sorry!
    _checkOption = (opt->check == 1);
    _configDir   = opt->config_dir;

    // Set up the translator.
    TSettings settings;
    Translator::initializeTranslator();
    global_translator->translateApp(
        this, settings.value("app/language", LANG_AUTO).toString());
}

AppCmdline::~AppCmdline()
{
    Translator::destroy();
}

bool AppCmdline::handle_init(const QList<struct init_info> &steps)
{
    // Handle each step of the initialization
    for(const struct init_info &info : steps)
    {
        if(!handle_step(info))
            return false;
    }

    // We don't have anything else to do
    return true;
}

/*
 * Show message(s) (if applicable), and return false if there's an error.
 *
 * In order for Qt's translation tr() to work, this must be a class method
 * (rather than a static function).
 */
bool AppCmdline::handle_step(const struct init_info &info)
{
    switch(info.status)
    {
    case INIT_OK:
        return true;
    case INIT_NEEDS_SETUP:
        DEBUG << tr("Cannot proceed without a config file.");
        return false;
    case INIT_DB_FAILED:
        DEBUG << tr("Cannot initialize the database.");
        return false;
    case INIT_DRY_RUN:
    case INIT_SCHEDULE_ERROR:
    case INIT_SETTINGS_RENAMED:
        DEBUG << info.message;
        // Don't return an error, because these aren't critical errors
        return true;
    case INIT_SCHEDULE_OK:
        // info.message contains longer text intended for a GUI message box.
        DEBUG << info.extra;
        return true;
    }

    // Should not happen
    DEBUG << "AppCmdline: unexpected info.status:" << info.status;
    return false;
}

bool AppCmdline::prepEventLoop()
{
    return false;
}

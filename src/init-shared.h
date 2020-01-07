#ifndef INIT_SHARED_H
#define INIT_SHARED_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

/*! \file init-shared.h
    \brief Shared init code that doesn't depend on the gui.
 */

//! Status of \ref init_shared_core().
enum init_status
{
    INIT_OK,
    INIT_SETTINGS_RENAMED,
    INIT_NEEDS_SETUP,
    INIT_DB_FAILED,
    INIT_DRY_RUN,
    INIT_SCHEDULE_OK,
    INIT_SCHEDULE_ERROR
};

//! Return value of \ref init_shared_core().
struct init_info
{
    //! Indicates any problems or actions required.
    init_status status;
    //! If there is a problem, what's the message?
    QString message;
    //! Extra string for special cases.  See code.
    QString extra;
};

/**
 * Constructor initialization shared between GUI and non-GUI.  Cannot fail.
 */
void init_shared_nofail();

/**
 * Configures the app-wide Settings.  Cannot fail, but can report a message.
 */
struct init_info init_shared_settings(QString configDir);

/**
 * Initialization shared between GUI and non-GUI.  Can fail and report messages.
 */
struct init_info init_shared_core();

#endif

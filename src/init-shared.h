#ifndef INIT_SHARED_H
#define INIT_SHARED_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QString>
WARNINGS_ENABLE

/*! \file init-shared.h
    \brief Shared init code that doesn't depend on the gui.
 */

//! Status of \ref init_shared().
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

//! Return value of \ref init_shared().
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
 * Initialization shared between GUI and non-GUI.
 * \return list a QList<struct init_info> with one element per
 * step of the initialization.
 */
QList<struct init_info> init_shared(const QString &configDir);

/**
 * Do we need to run the setup wizard?
 */
bool init_shared_need_setup();

/**
 * Free resources allocated in the init layer.
 */
void init_shared_free(void);

#endif

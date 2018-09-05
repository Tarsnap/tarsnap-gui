#ifndef INIT_CORE_H
#define INIT_CORE_H

#include <QCoreApplication>

/*! \file init-core.h
    \brief Shared init code that doesn't depend on the gui.
 */

enum init_status
{
    INIT_OK,
    INIT_NEEDS_SETUP,
    INIT_DRY_RUN,
    INIT_SCHEDULE_OK,
    INIT_SCHEDULE_ERROR
};

struct init_info
{
    init_status status;
    QString     message;
    QString     extra;
};

/**
 * Constructor initialization shared between GUI and non-GUI.  Cannot fail.
 */
void init_shared(QCoreApplication *app);

/**
 * Initialization shared between GUI and non-GUI.  Can fail and report messages.
 */
struct init_info init_shared_core(QCoreApplication *app);

#endif

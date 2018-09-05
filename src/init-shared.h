#ifndef INIT_CORE_H
#define INIT_CORE_H

#include <QCoreApplication>

/*! \file init-core.h
    \brief Shared init code that doesn't depend on the gui.
 */

/**
 * Initialization that doesn't depend on the gui.
 */
void init_shared(QCoreApplication *app);

#endif

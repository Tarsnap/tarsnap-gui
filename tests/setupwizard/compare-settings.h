#ifndef COMPARE_SETTINGS_H
#define COMPARE_SETTINGS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QSettings>
WARNINGS_ENABLE

//! Compares all the keys in two QSettings files.
bool compareSettings(QSettings *settings, QSettings *target);

#endif /* !COMPARE_SETTINGS_H */

#ifndef HUMANBYTES_H
#define HUMANBYTES_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

// Convert an int64 size in bytes to a human readable string using either
// SI(1000) or IEC(1024) units (default is SI) depending on the app setting
const QString humanBytes(quint64 bytes, int fieldWidth = 0);

#endif /* !HUMANBYTES_H */

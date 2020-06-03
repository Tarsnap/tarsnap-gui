#ifndef UPGRADE_STORE_H
#define UPGRADE_STORE_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QSqlDatabase;

bool upgrade_store(QSqlDatabase db, const QString &appdata);

#endif /* !UPGRADE_STORE_H */

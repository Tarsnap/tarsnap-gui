#include "basetask.h"

WARNINGS_DISABLE
#include <QUuid>
WARNINGS_ENABLE

BaseTask::BaseTask() : _uuid(QUuid::createUuid())
{
#ifdef QT_TESTLIB_LIB
    _fake = false;
#endif
}

QUuid BaseTask::uuid() const
{
    return (_uuid);
}

#ifdef QT_TESTLIB_LIB
void BaseTask::fake()
{
    _fake = true;
}
#endif

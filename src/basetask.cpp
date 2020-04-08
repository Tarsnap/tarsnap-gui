#include "basetask.h"

WARNINGS_DISABLE
#include <QObject>
WARNINGS_ENABLE

BaseTask::BaseTask() : QObject()
{
#ifdef QT_TESTLIB_LIB
    _fake = false;
#endif
}

BaseTask::~BaseTask()
{
}

#ifdef QT_TESTLIB_LIB
void BaseTask::fake()
{
    _fake = true;
}
#endif

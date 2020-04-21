#include "tasks/tasks-misc.h"

WARNINGS_DISABLE
#include <QStringList>
WARNINGS_ENABLE

#include "cmdlinetask.h"
#include "tasks/tasks-utils.h"

CmdlineTask *keyIdTask(const QString &key_filename)
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args;

    /* Specific arguments. */
    task->setCommand(makeTarsnapCommand("tarsnap-keymgmt"));
    args << "--print-key-id" << key_filename;

    /* Generic setup. */
    task->setArguments(args);
    return (task);
}

CmdlineTask *sleepSecondsTask(int seconds)
{
    CmdlineTask *task = new CmdlineTask();
    QStringList  args;

    /* Specific arguments. */
    task->setCommand("sleep");
    args << QString::number(seconds);

    /* Generic setup. */
    task->setArguments(args);
    return (task);
}

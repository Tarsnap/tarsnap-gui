#include "tasks-misc.h"

WARNINGS_DISABLE
#include <QStringList>
WARNINGS_ENABLE

#include "tasks-utils.h"

TarsnapTask *keyIdTask(const QString &key_filename)
{
    TarsnapTask *task = new TarsnapTask();
    QStringList  args;

    /* Specific arguments. */
    task->setCommand(makeTarsnapCommand("tarsnap-keymgmt"));
    args << "--print-key-id" << key_filename;

    /* Generic setup. */
    task->setArguments(args);
    return (task);
}

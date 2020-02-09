#include "tasks-setup.h"

WARNINGS_DISABLE
#include <QFileInfo>
#include <QRegExp>
WARNINGS_ENABLE

#include <TSettings.h>

#include "tarsnaptask.h"
#include "tasks-defs.h"
#include "tasks-utils.h"
#include "taskstatus.h"

TarsnapTask *tarsnapVersionTask()
{
    TarsnapTask *task = new TarsnapTask();
    QStringList  args;

    /* Specific arguments. */
    args << "--version";

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

const QString tarsnapVersionTaskParse(const QString &stdOut)
{
    QRegExp versionRx("^tarsnap (\\S+)\\s?$");
    if(-1 != versionRx.indexIn(stdOut))
        return (versionRx.cap(1));
    else
        return (QString(""));
}

TarsnapTask *registerMachineTask(const QString &password,
                                 const QString &machine)
{
    TarsnapTask *task = new TarsnapTask();
    QStringList  args;

    /*
     * Get settings.  We can't use the generic makeTarsnapArgs()
     * because it would be confused by options like --cachedir.
     */
    TSettings settings;
    QString   user        = settings.value("tarsnap/user", "").toString();
    QString   keyFilename = settings.value("tarsnap/key", "").toString();

    /* Specific arguments. */
    args << "--user" << user << "--machine" << machine << "--keyfile"
         << keyFilename;
    task->setStdIn(password);

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand("tarsnap-keygen"));
    task->setArguments(args);
    return (task);
}

TarsnapTask *fsckTask(bool prune)
{
    TarsnapTask *task = new TarsnapTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    if(prune)
        args << "--fsck-prune";
    else
        args << "--fsck";

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

TarsnapTask *initializeCachedirTask()
{
    TarsnapTask *task = new TarsnapTask();
    QStringList  args = makeTarsnapArgs();

    /* Specific arguments. */
    args << "--initialize-cachedir";

    /* Generic setup. */
    task->setCommand(makeTarsnapCommand());
    task->setArguments(args);
    return (task);
}

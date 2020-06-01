#include "dir-utils.h"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStringList>
WARNINGS_ENABLE

#include "tasks/tasks-defs.h"

const QString validate_writeable_dir(const QString &dirname)
{
    if(dirname.isEmpty())
        return QCoreApplication::translate("dir-utils", "Path is empty.");

    QFileInfo candidate(dirname);
    if(!candidate.exists())
        return QCoreApplication::translate("dir-utils", "Does not exist.");

    if(!candidate.isDir())
        return QCoreApplication::translate("dir-utils", "Not a directory.");

    if(!candidate.isWritable())
        return QCoreApplication::translate("dir-utils", "Not writable.");

    // We're ok.
    return "";
}

const QString validate_readable_file(const QString &filename)
{
    if(filename.isEmpty())
        return QCoreApplication::translate("dir-utils", "Path is empty.");

    QFileInfo candidate(filename);
    if(!candidate.exists())
        return QCoreApplication::translate("dir-utils", "Does not exist.");

    if(!candidate.isFile())
        return QCoreApplication::translate("dir-utils", "Not a file.");

    if(!candidate.isReadable())
        return QCoreApplication::translate("dir-utils", "Not readable.");

    // We're ok.
    return "";
}

static QString validate_executable(const QString &executable)
{
    if(executable.isEmpty())
        return QCoreApplication::translate("dir-utils",
                                           "Executable not found.");

    // I don't think that the following checks are neded, as it appears that
    // QStandardPaths::findExecutable() won't return a non-executable file
    // (quite sensibly).  I considered deleting the checks, and probably this
    // entire function, but it's possible that we'll refactor to avoid using
    // ::findExecutable(), in which case these lines would become relevant.

    QFileInfo info(executable);
    if(!info.isReadable())
        return QCoreApplication::translate("dir-utils", "Not readable.");

    if(!info.isExecutable())
        return QCoreApplication::translate("dir-utils", "Not executable.");

    // We're ok.
    return "";
}

static struct DirMessage findBinary(const QString &cmd, QStringList searchPaths)
{
    struct DirMessage result;

    QString executable;

    executable = QStandardPaths::findExecutable(cmd, searchPaths);
#if defined(Q_OS_OSX)
    // If we haven't found the command in the default PATH, look
    // in /usr/local/bin because that's where brew puts it.
    QStringList brew_bin = {"/usr/local/bin"};
    if(executable.isEmpty() && searchPaths.isEmpty())
        executable = QStandardPaths::findExecutable(CMD_TARSNAP, brew_bin);
#endif
    result.errorMessage = validate_executable(executable);
    if(!result.errorMessage.isEmpty())
        return result;

    result.dirname = executable;
    return result;
}

struct DirMessage findTarsnapClientInPath(const QString &path, bool keygenToo)
{
    struct DirMessage result;

    QStringList searchPaths;

    if(!path.isEmpty())
        searchPaths << path;

    // Look for main tarsnap binary.
    result = findBinary(CMD_TARSNAP, searchPaths);
    if(result.dirname.isEmpty())
        return result;

    // Look for tarsnap-keygen.
    if(keygenToo)
    {
        result = findBinary(CMD_TARSNAPKEYGEN, searchPaths);
        if(result.dirname.isEmpty())
            return result;
    }

    // If we were searching $PATH, store the directory.
    if(path.isEmpty())
        result.dirname = QFileInfo(result.dirname).absolutePath();
    else
        result.dirname = path;

    return result;
}

QFileInfoList findKeysInPath(const QString &path)
{
    QFileInfoList result;
    QDir          dir(path);
    if(!dir.exists())
        return result;
    dir.setFilter(QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Time);
    dir.setNameFilters(QStringList("*.key"));
    return dir.entryInfoList();
}

#include "utils.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
WARNINGS_ENABLE

#include <math.h>

#include <TSettings.h>

#include "tasks-defs.h"
#include "tasks-utils.h"

using namespace Utils;

GetDirInfoTask::GetDirInfoTask(QDir dir) : _dir(dir)
{
}

void GetDirInfoTask::run()
{
    quint64 size  = 0;
    quint64 count = 0;

    size  = getDirSize(_dir);
    count = getDirCount(_dir);

    emit result(size, count);
}

quint64 GetDirInfoTask::getDirSize(QDir dir)
{
    quint64 size = 0;
    if(dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot
                      | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for(int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                size += getDirSize(QDir(fileInfo.absoluteFilePath()));
            }
            else
                size += static_cast<quint64>(fileInfo.size());
        }
    }
    return size;
}

quint64 GetDirInfoTask::getDirCount(QDir dir)
{
    quint64 count = 0;
    if(dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot
                      | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for(int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                count += getDirCount(QDir(fileInfo.absoluteFilePath()));
            }
            ++count;
        }
    }
    return count;
}

QString Utils::humanBytes(quint64 bytes, int fieldWidth)
{
    TSettings settings;
    bool      IEC  = settings.value("app/iec_prefixes", false).toBool();
    quint64   unit = IEC ? 1024 : 1000;
    if(bytes < unit)
        return QString::number(bytes) + " B";
    int     exp = static_cast<int>(log(bytes) / log(unit));
    QString pre = QString(IEC ? "KMGTPE" : "kMGTPE").at(exp - 1)
                  + QString(IEC ? "i" : "");
    return QString("%1 %2B")
        .arg(static_cast<double>(bytes) / pow(unit, exp), fieldWidth, 'f', 2)
        .arg(pre);
}

const QString Utils::validate_writeable_dir(const QString &dirname)
{
    if(dirname.isEmpty())
        return QCoreApplication::translate("Utils", "Path is empty.");

    QFileInfo candidate(dirname);
    if(!candidate.exists())
        return QCoreApplication::translate("Utils", "Does not exist.");

    if(!candidate.isDir())
        return QCoreApplication::translate("Utils", "Not a directory.");

    if(!candidate.isWritable())
        return QCoreApplication::translate("Utils", "Not writable.");

    // We're ok.
    return "";
}

const QString Utils::validate_readable_file(const QString &filename)
{
    if(filename.isEmpty())
        return QCoreApplication::translate("Utils", "Path is empty.");

    QFileInfo candidate(filename);
    if(!candidate.exists())
        return QCoreApplication::translate("Utils", "Does not exist.");

    if(!candidate.isFile())
        return QCoreApplication::translate("Utils", "Not a file.");

    if(!candidate.isReadable())
        return QCoreApplication::translate("Utils", "Not readable.");

    // We're ok.
    return "";
}

static QString validate_executable(const QString &executable)
{
    if(executable.isEmpty())
        return QCoreApplication::translate("Utils", "Executable not found.");

    // I don't think that the following checks are neded, as it appears that
    // QStandardPaths::findExecutable() won't return a non-executable file
    // (quite sensibly).  I considered deleting the checks, and probably this
    // entire function, but it's possible that we'll refactor to avoid using
    // ::findExecutable(), in which case these lines would become relevant.

    QFileInfo info(executable);
    if(!info.isReadable())
        return QCoreApplication::translate("Utils", "Not readable.");

    if(!info.isExecutable())
        return QCoreApplication::translate("Utils", "Not executable.");

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

struct DirMessage Utils::findTarsnapClientInPath(QString path, bool keygenToo)
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

    // If we were searching $PATH, update the `path` argument.
    if(path.isEmpty())
        path = QFileInfo(result.dirname).absolutePath();

    result.dirname = path;
    return result;
}

QFileInfoList Utils::findKeysInPath(QString path)
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

QString Utils::quoteCommandLine(QStringList args)
{
    QStringList escaped;
    QRegExp     rx("^[0-9a-z-A-Z/._-]*$");
    QString     cmdLine;

    for(int i = 0; i < args.size(); ++i)
    {
        QString arg = args.at(i);
        if(rx.indexIn(arg) >= 0)
        {
            escaped.append(arg);
        }
        else
        {
            escaped.append(arg.prepend("\'").append("\'"));
        }
    }

    cmdLine = escaped.join(' ');
    return (cmdLine);
}

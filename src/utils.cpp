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
        .arg(bytes / pow(unit, exp), fieldWidth, 'f', 2)
        .arg(pre);
}

QString Utils::validateTarsnapCache(QString path)
{
    QString result;
    if(!path.isEmpty())
    {
        QFileInfo candidate(path);
        if(candidate.exists() && candidate.isDir() && candidate.isWritable())
            result = candidate.canonicalFilePath();
    }
    return result;
}

QString Utils::validateAppDataDir(QString path)
{
    QString result;
    if(!path.isEmpty())
    {
        QFileInfo candidate(path);
        if(candidate.exists() && candidate.isDir() && candidate.isWritable())
            result = candidate.canonicalFilePath();
    }
    return result;
}

static bool validate_executable(const QString &executable)
{
    if(executable.isEmpty())
        return false;

    QFileInfo info(executable);
    if(!info.isReadable())
        return false;

    if(!info.isExecutable())
        return false;

    // We're ok.
    return true;
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
    if(!validate_executable(executable))
        return result;

    result.dirname = executable;
    return result;
}

QString Utils::findTarsnapClientInPath(QString path, bool keygenToo)
{
    struct DirMessage result;

    QStringList searchPaths;

    if(!path.isEmpty())
        searchPaths << path;

    // Look for main tarsnap binary.
    result = findBinary(CMD_TARSNAP, searchPaths);
    if(result.dirname.isEmpty())
        return "";

    // Look for tarsnap-keygen.
    if(keygenToo)
    {
        result = findBinary(CMD_TARSNAPKEYGEN, searchPaths);
        if(result.dirname.isEmpty())
            return "";
    }

    // If we were searching $PATH, update the `path` argument.
    if(path.isEmpty())
        path = QFileInfo(result.dirname).absolutePath();

    return path;
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

bool Utils::tarsnapVersionMinimum(const QString &minVersion)
{
    TSettings settings;
    QString   tarsnapVersion = settings.value("tarsnap/version", "").toString();
    // A blank version string doesn't satisfy any requirement.
    if(tarsnapVersion.isEmpty())
        return false;
    // This function will return -1, 0, or 1; we want anything other than -1.
    return (versionCompare(tarsnapVersion, minVersion) >= 0);
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

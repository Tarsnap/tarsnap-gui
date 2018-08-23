#include "utils.h"

#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

#include <math.h>

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
    QSettings settings;
    bool      IEC  = settings.value("app/iec_prefixes", false).toBool();
    quint64   unit = IEC ? 1024 : 1000;
    if(bytes < unit)
        return QString::number(bytes) + " B";
    int     exp = static_cast<int>(log(bytes) / log(unit));
    QString pre =
        QString(IEC ? "KMGTPE" : "kMGTPE").at(exp - 1) + QString(IEC ? "i" : "");
    return QString("%1 %2B").arg(bytes / pow(unit, exp), fieldWidth, 'f', 2).arg(pre);
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

QString Utils::findTarsnapClientInPath(QString path, bool keygenToo)
{
    QStringList searchPaths;
    QString     executable;

    if(!path.isEmpty())
        searchPaths << path;

    executable = QStandardPaths::findExecutable(CMD_TARSNAP, searchPaths);
    if(executable.isEmpty() || !QFileInfo(executable).isReadable()
       || !QFileInfo(executable).isExecutable())
        return "";
    else if(keygenToo)
    {
        executable =
            QStandardPaths::findExecutable(CMD_TARSNAPKEYGEN, searchPaths);
    }

    if(executable.isEmpty() || !QFileInfo(executable).isReadable()
       || !QFileInfo(executable).isExecutable())
        return "";
    else if(path.isEmpty())
        path = QFileInfo(executable).absolutePath();

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
    QSettings settings;
    QString   tarsnapVersion = settings.value("tarsnap/version", "").toString();
    QRegExp   versionRx("(\\d+\\.\\d+\\.\\d+(\\.\\d+)?)");
    return (-1 != versionRx.indexIn(tarsnapVersion))
           && (versionRx.cap(0) >= minVersion);
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

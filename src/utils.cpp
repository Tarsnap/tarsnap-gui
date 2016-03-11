#include "utils.h"

#include <QDebug>
#include <QStandardPaths>
#include <QSettings>

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
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot |
                      QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for(int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
//                qDebug() << "Traversing " << fileInfo.absoluteFilePath();
                size += getDirSize(QDir(fileInfo.absoluteFilePath()));
            }
            else
                size += fileInfo.size();
        }
    }
    return size;
}

quint64 GetDirInfoTask::getDirCount(QDir dir)
{
    quint64 count = 0;
    if(dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot |
                      QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for(int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
//                qDebug() << "Traversing " << fileInfo.absoluteFilePath();
                count += getDirCount(QDir(fileInfo.absoluteFilePath()));
            }
            ++count;
        }
    }
    return count;
}

QString Utils::humanBytes(quint64 bytes, bool IEC, int fieldWidth)
{
    quint64 unit = IEC ? 1024 : 1000;
    if(bytes < unit)
        return QString::number(bytes) + " B";
    int     exp = (int)(log(bytes) / log(unit));
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

QString Utils::findTarsnapClientInPath(QString path, bool keygenToo)
{
    QStringList paths;

    if(!path.isEmpty())
        paths << path;

    QString result = QStandardPaths::findExecutable(CMD_TARSNAP, paths);
    if(!result.isEmpty() && keygenToo)
        result = QStandardPaths::findExecutable(CMD_TARSNAPKEYGEN, paths);

    if(result.isEmpty())
        return result;
    else if(path.isEmpty())
        path = QFileInfo(result).absolutePath();

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
    QString tarsnapVersion = settings.value("tarsnap/version", "").toString();
    QRegExp versionRx("(\\d+\\.\\d+\\.\\d+(\\.\\d+)?)");
    return (-1 != versionRx.indexIn(tarsnapVersion)) && (versionRx.cap(0) >= minVersion);
}

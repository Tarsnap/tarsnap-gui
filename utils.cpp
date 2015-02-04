#include "utils.h"

#include <math.h>
#include <QDebug>

using namespace Utils;

GetDirInfoTask::GetDirInfoTask(QDir dir):
    _dir(dir)
{

}

void GetDirInfoTask::run()
{
    qint64 size = 0;
    qint64 count = 0;

    size = getDirSize(_dir);
    count = getDirCount(_dir);

    emit result(size, count);
}

qint64 GetDirInfoTask::getDirSize(QDir dir)
{
    qint64 size = 0;
    if(dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
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

qint64 GetDirInfoTask::getDirCount(QDir dir)
{
    qint64 count = 0;
    if(dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
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

QString Utils::humanBytes(qint64 bytes, bool si)
{
    int unit = si ? 1000 : 1024;
    if (bytes < unit) return QString::number(bytes) + " B";
    int exp = (int) (log(bytes) / log(unit));
    QString pre = QString(si ? "kMGTPE" : "KMGTPE").at(exp-1) + QString(si ? "" : "i");
    return QString("%1 %2B").arg(bytes / pow(unit, exp), 0, 'f', 1).arg(pre);
}

#include "utils.h"

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
                qDebug() << "Traversing " << fileInfo.absoluteFilePath();
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
                qDebug() << "Traversing " << fileInfo.absoluteFilePath();
                count += getDirCount(QDir(fileInfo.absoluteFilePath()));
            }
            ++count;
        }
    }
    return count;
}

#include "dirinfotask.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QStandardPaths>
WARNINGS_ENABLE

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

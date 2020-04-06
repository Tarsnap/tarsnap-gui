#include "dirinfotask.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QStandardPaths>
WARNINGS_ENABLE

/** Info about a directory (used recursively). */
struct dirinfo
{
    /** Sum of file sizes. */
    quint64 size;
    /** Number of files. */
    quint64 count;
};

/* Forward declaration. */
struct dirinfo getDirInfo(const QDir dir);

DirInfoTask::DirInfoTask(QDir dir) : _dir(dir)
{
}

void DirInfoTask::run()
{
    struct dirinfo dirinfo = getDirInfo(_dir);
    emit           result(dirinfo.size, dirinfo.count);
}

struct dirinfo getDirInfo(QDir dir)
{
    struct dirinfo dirinfo = {0, 0};
    if(dir.exists())
    {
        // We want to see all directories and files, no symlinks,
        // and no . and .. directories.
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot
                      | QDir::Hidden | QDir::NoSymLinks);

        // For each directory item...
        QFileInfoList list = dir.entryInfoList();
        for(int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                // ... if it's a dir, recursively get info about that dir.
                struct dirinfo d = getDirInfo(fileInfo.absoluteFilePath());
                dirinfo.size += d.size;
                dirinfo.count += d.count;
            }
            else
            {
                // ... if it's a file, add its info to the total.
                dirinfo.size += static_cast<quint64>(fileInfo.size());
                dirinfo.count++;
            }
        }
    }
    return dirinfo;
}

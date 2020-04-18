#include "dirinfotask.h"

WARNINGS_DISABLE
#include <QAtomicInt>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFlags>
WARNINGS_ENABLE

#include "basetask.h"

/** Info about a directory (used recursively). */
struct dirinfo
{
    /** Sum of file sizes. */
    quint64 size;
    /** Number of files. */
    quint64 count;
};

/* Forward declaration(s). */
struct dirinfo getDirInfo(const QDir dir, QAtomicInt *stop_p);

DirInfoTask::DirInfoTask(QDir dir) : BaseTask(), _dir(dir)
{
}

void DirInfoTask::run()
{
    struct dirinfo dirinfo = getDirInfo(_dir, &_stopRequested);

    // Send appropriate notification.
    if(static_cast<int>(_stopRequested) == 1)
        emit canceled();
    else
        emit result(dirinfo.size, dirinfo.count);

    // We're finished.
    emit dequeue();
}

void DirInfoTask::stop()
{
    _stopRequested = 1;
}

struct dirinfo getDirInfo(QDir dir, QAtomicInt *stop_p)
{
    struct dirinfo dirinfo = {0, 0};

    // Bail if requested.
    if(static_cast<int>(*stop_p) == 1)
        return dirinfo;

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
                struct dirinfo d =
                    getDirInfo(fileInfo.absoluteFilePath(), stop_p);
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

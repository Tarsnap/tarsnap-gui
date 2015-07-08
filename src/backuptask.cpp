#include "backuptask.h"

#include <QFileInfo>

#define MB 1048576

BackupTask::BackupTask():_uuid(QUuid::createUuid()), _optionPreservePaths(true), _optionTraverseMount(true),
    _optionFollowSymLinks(false), _optionSkipFilesSize(0), _status(TaskStatus::Initialized)
{
    QSettings settings;
    _optionPreservePaths          = settings.value("tarsnap/preserve_pathnames", true).toBool();
    _optionTraverseMount          = settings.value("tarsnap/traverse_mount", true).toBool();
    _optionFollowSymLinks         = settings.value("tarsnap/follow_symlinks", false).toBool();
    _optionSkipFilesSize          = MB * settings.value("app/skip_files_value", 0).toLongLong();
}

BackupTask::~BackupTask()
{

}

bool BackupTask::optionPreservePaths() const
{
    return _optionPreservePaths;
}

void BackupTask::setOptionPreservePaths(bool optionPreservePaths)
{
    _optionPreservePaths = optionPreservePaths;
}

quint64 BackupTask::optionSkipFilesSize() const
{
    return _optionSkipFilesSize;
}

void BackupTask::setOptionSkipFilesSize(const quint64 &optionSkipFilesSize)
{
    _optionSkipFilesSize = MB * optionSkipFilesSize;
}

bool BackupTask::optionFollowSymLinks() const
{
    return _optionFollowSymLinks;
}

void BackupTask::setOptionFollowSymLinks(bool optionFollowSymLinks)
{
    _optionFollowSymLinks = optionFollowSymLinks;
}

bool BackupTask::optionTraverseMount() const
{
    return _optionTraverseMount;
}

void BackupTask::setOptionTraverseMount(bool optionTraverseMount)
{
    _optionTraverseMount = optionTraverseMount;
}

QStringList BackupTask::getExcludesList()
{
    QStringList skipList;

    if(_optionSkipFilesSize)
    {
        foreach (QUrl url, urls()) {
            QFileInfo file(url.toLocalFile());
            if(file.isFile())
            {
                if(quint64(file.size()) >= _optionSkipFilesSize)
                {
                    skipList << QRegExp::escape(url.toLocalFile());
                }
            }
            else if(file.isDir())
            {
                QStack<QDir> dirStack;
                dirStack.push(QDir(file.absoluteFilePath()));
                while (!dirStack.isEmpty())
                {
                    QDir dir(dirStack.pop());
                    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks);
                    foreach (QFileInfo entry, dir.entryInfoList())
                    {
                        if(entry.isFile())
                        {
                            if(quint64(entry.size()) >= _optionSkipFilesSize)
                                skipList << QRegExp::escape(entry.absoluteFilePath());
                        }
                        else if(entry.isDir())
                        {
                            dirStack.push(QDir(entry.absoluteFilePath()));
                        }
                    }
                }
            }
        }
    }

    return skipList;
}

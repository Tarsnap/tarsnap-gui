#include "backuptask.h"

#include <QFileInfo>

#define MB 1048576

BackupTask::BackupTask(): _uuid(QUuid::createUuid()),
    _timestamp(QDateTime::currentDateTime()), _optionPreservePaths(true),
    _optionTraverseMount(true), _optionFollowSymLinks(false),
    _optionSkipFilesSize(0), _optionSkipSystem(false),
    _optionSkipSystemFiles(), _status(TaskStatus::Initialized)
{
    QSettings settings;
    setOptionPreservePaths(settings.value("tarsnap/preserve_pathnames", true).toBool());
    setOptionTraverseMount(settings.value("tarsnap/traverse_mount", true).toBool());
    setOptionFollowSymLinks(settings.value("tarsnap/follow_symlinks", false).toBool());
    setOptionSkipFilesSize(settings.value("app/skip_files_size", 0).toULongLong());
    setOptionSkipSystem(settings.value("app/skip_system_enabled", false).toBool());
    setOptionSkipSystemFiles(settings.value("app/skip_system_files", "").toString());
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

bool BackupTask::optionSkipSystem() const
{
    return _optionSkipSystem;
}

void BackupTask::setOptionSkipSystem(bool optionSkipSystem)
{
    _optionSkipSystem = optionSkipSystem;
}

QStringList BackupTask::optionSkipSystemFiles() const
{
    return _optionSkipSystemFiles;
}

void BackupTask::setOptionSkipSystemFiles(const QStringList &optionSkipSystemFiles)
{
    _optionSkipSystemFiles = optionSkipSystemFiles;
}

void BackupTask::setOptionSkipSystemFiles(const QString string)
{
    _optionSkipSystemFiles = string.split(':', QString::SkipEmptyParts);
}

QStringList BackupTask::getExcludesList()
{
    QStringList skipList;

    if(_optionSkipSystem)
    {
        skipList.append(_optionSkipSystemFiles);
    }

    if(_optionSkipFilesSize)
    {
        foreach(QUrl url, urls())
        {
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
                    foreach(QFileInfo entry, dir.entryInfoList())
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

bool BackupTask::optionDryRun() const
{
    return _optionDryRun;
}

void BackupTask::setOptionDryRun(bool optionDryRun)
{
    _optionDryRun = optionDryRun;
}

bool BackupTask::optionSkipNoDump() const
{
    return _optionSkipNoDump;
}

void BackupTask::setOptionSkipNoDump(bool optionSkipNoDump)
{
    _optionSkipNoDump = optionSkipNoDump;
}

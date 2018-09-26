#include "backuptask.h"

#include "utils.h"

#include <QFileInfo>

#include <TSettings.h>

#define MB 1048576ULL

BackupTask::BackupTask()
    : _uuid(QUuid::createUuid()),
      _timestamp(QDateTime::currentDateTime()),
      _optionPreservePaths(DEFAULT_PRESERVE_PATHNAMES),
      _optionTraverseMount(DEFAULT_TRAVERSE_MOUNT),
      _optionFollowSymLinks(DEFAULT_FOLLOW_SYMLINKS),
      _optionSkipFilesSize(DEFAULT_SKIP_FILES_SIZE),
      _optionSkipSystem(DEFAULT_SKIP_SYSTEM_ENABLED),
      _optionSkipSystemFiles(DEFAULT_SKIP_SYSTEM_FILES),
      _optionDryRun(DEFAULT_DRY_RUN),
      _optionSkipNoDump(DEFAULT_SKIP_NODUMP),
      _status(TaskStatus::Initialized),
      _exitCode(0)
{
    TSettings settings;
    setOptionPreservePaths(
        settings.value("tarsnap/preserve_pathnames", optionPreservePaths())
            .toBool());
    setOptionTraverseMount(
        settings.value("tarsnap/traverse_mount", optionTraverseMount()).toBool());
    setOptionFollowSymLinks(
        settings.value("tarsnap/follow_symlinks", optionFollowSymLinks()).toBool());
    setOptionDryRun(settings.value("tarsnap/dry_run", optionDryRun()).toBool());
    setOptionSkipNoDump(
        settings.value("app/skip_nodump", optionSkipNoDump()).toBool());
    setOptionSkipFilesSize(
        settings.value("app/skip_files_size", optionSkipFilesSize()).toInt());
    setOptionSkipSystem(
        settings.value("app/skip_system_enabled", optionSkipSystem()).toBool());
    setOptionSkipSystemFiles(
        settings.value("app/skip_system_files", optionSkipSystemFiles())
            .toString());
}

QString BackupTask::name() const
{
    return _name;
}

void BackupTask::setName(const QString &name)
{
    _name = name;
}

QUuid BackupTask::uuid() const
{
    return _uuid;
}

void BackupTask::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}

QDateTime BackupTask::timestamp() const
{
    return _timestamp;
}

void BackupTask::setTimestamp(const QDateTime &timestamp)
{
    _timestamp = timestamp;
}

QString BackupTask::jobRef() const
{
    return _jobRef;
}

void BackupTask::setJobRef(const QString &jobRef)
{
    _jobRef = jobRef;
}

QList<QUrl> BackupTask::urls() const
{
    return _urls;
}

void BackupTask::setUrls(const QList<QUrl> &urls)
{
    _urls = urls;
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

void BackupTask::setOptionSkipFilesSize(const int &optionSkipFilesSize)
{
    _optionSkipFilesSize = MB * static_cast<quint64>(optionSkipFilesSize);
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
                while(!dirStack.isEmpty())
                {
                    QDir dir(dirStack.pop());
                    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot
                                  | QDir::Hidden | QDir::NoSymLinks);
                    foreach(QFileInfo entry, dir.entryInfoList())
                    {
                        if(entry.isFile())
                        {
                            if(quint64(entry.size()) >= _optionSkipFilesSize)
                                skipList
                                    << QRegExp::escape(entry.absoluteFilePath());
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

TaskStatus BackupTask::status() const
{
    return _status;
}

void BackupTask::setStatus(const TaskStatus &status)
{
    _status = status;
    emit statusUpdate(_uuid, _status);
}

int BackupTask::exitCode() const
{
    return _exitCode;
}

void BackupTask::setExitCode(int exitCode)
{
    _exitCode = exitCode;
}

QString BackupTask::output() const
{
    return _output;
}

void BackupTask::setOutput(const QString &output)
{
    _output = output;
}

ArchivePtr BackupTask::archive() const
{
    return _archive;
}

void BackupTask::setArchive(const ArchivePtr &archive)
{
    _archive = archive;
}

QString BackupTask::command() const
{
    return _command;
}

void BackupTask::setCommand(const QString &command)
{
    _command = command;
}

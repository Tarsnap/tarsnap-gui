#include "backuptask.h"

WARNINGS_DISABLE
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFlags>
#include <QList>
#include <QRegExp>
#include <QStack>
#include <QUrl>
#include <QVariant>
WARNINGS_ENABLE

#include "TSettings.h"

#include "compat.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "tasks/tasks-defs.h"

const quint64 MB = 1048576UL;

BackupTaskData::BackupTaskData()
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
      _exitCode(0)
{
    TSettings settings;
    setOptionPreservePaths(
        settings.value("tarsnap/preserve_pathnames", optionPreservePaths())
            .toBool());
    setOptionTraverseMount(
        settings.value("tarsnap/traverse_mount", optionTraverseMount())
            .toBool());
    setOptionFollowSymLinks(
        settings.value("tarsnap/follow_symlinks", optionFollowSymLinks())
            .toBool());
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

QString BackupTaskData::name() const
{
    return _name;
}

void BackupTaskData::setName(const QString &name)
{
    _name = name;
}

QUuid BackupTaskData::uuid() const
{
    return _uuid;
}

void BackupTaskData::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}

QDateTime BackupTaskData::timestamp() const
{
    return _timestamp;
}

void BackupTaskData::setTimestamp(const QDateTime &timestamp)
{
    _timestamp = timestamp;
}

QString BackupTaskData::jobRef() const
{
    return _jobRef;
}

void BackupTaskData::setJobRef(const QString &jobRef)
{
    _jobRef = jobRef;
}

QList<QUrl> BackupTaskData::urls() const
{
    return _urls;
}

void BackupTaskData::setUrls(const QList<QUrl> &urls)
{
    _urls = urls;
}

bool BackupTaskData::optionPreservePaths() const
{
    return _optionPreservePaths;
}

void BackupTaskData::setOptionPreservePaths(bool optionPreservePaths)
{
    _optionPreservePaths = optionPreservePaths;
}

quint64 BackupTaskData::optionSkipFilesSize() const
{
    return _optionSkipFilesSize;
}

void BackupTaskData::setOptionSkipFilesSize(const int &optionSkipFilesSize)
{
    _optionSkipFilesSize = MB * static_cast<quint64>(optionSkipFilesSize);
}

bool BackupTaskData::optionFollowSymLinks() const
{
    return _optionFollowSymLinks;
}

void BackupTaskData::setOptionFollowSymLinks(bool optionFollowSymLinks)
{
    _optionFollowSymLinks = optionFollowSymLinks;
}

bool BackupTaskData::optionTraverseMount() const
{
    return _optionTraverseMount;
}

void BackupTaskData::setOptionTraverseMount(bool optionTraverseMount)
{
    _optionTraverseMount = optionTraverseMount;
}

bool BackupTaskData::optionSkipSystem() const
{
    return _optionSkipSystem;
}

void BackupTaskData::setOptionSkipSystem(bool optionSkipSystem)
{
    _optionSkipSystem = optionSkipSystem;
}

QStringList BackupTaskData::optionSkipSystemFiles() const
{
    return _optionSkipSystemFiles;
}

void BackupTaskData::setOptionSkipSystemFiles(
    const QStringList &optionSkipSystemFiles)
{
    _optionSkipSystemFiles = optionSkipSystemFiles;
}

void BackupTaskData::setOptionSkipSystemFiles(const QString &string)
{
    _optionSkipSystemFiles = string.split(':', SKIP_EMPTY_PARTS);
}

QStringList BackupTaskData::getExcludesList()
{
    QStringList skipList;

    if(_optionSkipSystem)
    {
        skipList.append(_optionSkipSystemFiles);
    }

    if(_optionSkipFilesSize)
    {
        for(const QUrl &url : urls())
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
                    dir.setFilter(QDir::Dirs | QDir::Files
                                  | QDir::NoDotAndDotDot | QDir::Hidden
                                  | QDir::NoSymLinks);
                    // I'm not 100% certain that the below loop doesn't
                    // modify this value, so I'm making a copy to be safe.
                    const QFileInfoList entryInfoList = dir.entryInfoList();
                    for(const QFileInfo &entry : entryInfoList)
                    {
                        if(entry.isFile())
                        {
                            if(quint64(entry.size()) >= _optionSkipFilesSize)
                                skipList << QRegExp::escape(
                                    entry.absoluteFilePath());
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

bool BackupTaskData::optionDryRun() const
{
    return _optionDryRun;
}

void BackupTaskData::setOptionDryRun(bool optionDryRun)
{
    _optionDryRun = optionDryRun;
}

bool BackupTaskData::optionSkipNoDump() const
{
    return _optionSkipNoDump;
}

void BackupTaskData::setOptionSkipNoDump(bool optionSkipNoDump)
{
    _optionSkipNoDump = optionSkipNoDump;
}

int BackupTaskData::exitCode() const
{
    return _exitCode;
}

void BackupTaskData::setExitCode(int exitCode)
{
    _exitCode = exitCode;
}

QString BackupTaskData::output() const
{
    return _output;
}

void BackupTaskData::setOutput(const QString &output)
{
    _output = output;
}

ArchivePtr BackupTaskData::archive() const
{
    return _archive;
}

void BackupTaskData::setArchive(const ArchivePtr &archive)
{
    _archive = archive;
}

QString BackupTaskData::command() const
{
    return _command;
}

void BackupTaskData::setCommand(const QString &command)
{
    _command = command;
}

BackupTaskDataPtr BackupTaskData::createBackupTaskFromJob(JobPtr job)
{
    BackupTaskDataPtr backup(new BackupTaskData);
    backup->setName(
        JOB_NAME_PREFIX + job->name()
        + QDateTime::currentDateTime().toString(ARCHIVE_TIMESTAMP_FORMAT));
    backup->setJobRef(job->objectKey());
    backup->setUrls(job->urls());
    backup->setOptionPreservePaths(job->optionPreservePaths());
    backup->setOptionTraverseMount(job->optionTraverseMount());
    backup->setOptionFollowSymLinks(job->optionFollowSymLinks());
    backup->setOptionSkipFilesSize(job->optionSkipFilesSize());
    backup->setOptionSkipSystem(job->optionSkipFiles());
    backup->setOptionSkipSystemFiles(job->optionSkipFilesPatterns());
    backup->setOptionSkipNoDump(job->optionSkipNoDump());
    return backup;
}

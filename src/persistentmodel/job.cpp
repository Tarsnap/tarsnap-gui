#include "persistentmodel/job.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

#include <algorithm>

#include "TSettings.h"

#include "compat.h"
#include "debug.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/persistentstore.h"
#include "tasks/tasks-defs.h"

Job::Job(QObject *parent)
    : PersistentObject(parent),
      _optionScheduledEnabled(JobSchedule::Disabled),
      _optionPreservePaths(DEFAULT_PRESERVE_PATHNAMES),
      _optionTraverseMount(DEFAULT_TRAVERSE_MOUNT),
      _optionFollowSymLinks(DEFAULT_FOLLOW_SYMLINKS),
      _optionSkipFilesSize(DEFAULT_SKIP_FILES_SIZE),
      _optionSkipFiles(DEFAULT_SKIP_SYSTEM_ENABLED),
      _optionSkipFilesPatterns(DEFAULT_SKIP_SYSTEM_FILES),
      _optionSkipNoDump(DEFAULT_SKIP_NODUMP),
      _settingShowHidden(false),
      _settingShowSystem(false),
      _settingHideSymlinks(false),
      _fsWatcher(new QFileSystemWatcher(this))
{
    // Load values from settings.
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
    setOptionSkipNoDump(
        settings.value("app/skip_nodump", optionSkipNoDump()).toBool());
    setOptionSkipFilesSize(
        settings.value("app/skip_files_size", optionSkipFilesSize()).toInt());
    setOptionSkipFiles(
        settings.value("app/skip_system_enabled", optionSkipFiles()).toBool());
    setOptionSkipFilesPatterns(
        settings.value("app/skip_system_files", optionSkipFilesPatterns())
            .toString());
}

QString Job::name() const
{
    return _name;
}

void Job::setName(const QString &name)
{
    _name = name;
}

QString Job::archivePrefix()
{
    return JOB_NAME_PREFIX + name();
}

QList<QUrl> Job::urls() const
{
    return _urls;
}

void Job::setUrls(const QList<QUrl> &urls)
{
    _urls = urls;
}

bool Job::validateUrls()
{
    // Check that _urls is not empty.
    if(_urls.isEmpty())
        return false;
    // Check that every file exists.
    for(const QUrl &url : _urls)
    {
        QFileInfo file(url.toLocalFile());
        if(!file.exists())
            return false;
    }
    return true;
}

void Job::installWatcher()
{
    connect(_fsWatcher, &QFileSystemWatcher::directoryChanged, this,
            &Job::fsEvent);
    connect(_fsWatcher, &QFileSystemWatcher::fileChanged, this, &Job::fsEvent);

    for(const QUrl &url : _urls)
    {
        // Emit a signal if a file has changed.
        QFileInfo file(url.toLocalFile());
        _fsWatcher->addPath(file.absoluteFilePath());
        // Emit a signal if a directory or any of its ancestors has changed.
        QDir dir(file.absoluteDir());
        while(dir != QDir::root())
        {
            _fsWatcher->addPath(dir.absolutePath());
            dir.cdUp();
        }
    }
}

void Job::removeWatcher()
{
    disconnect(_fsWatcher, &QFileSystemWatcher::directoryChanged, this,
               &Job::fsEvent);
    disconnect(_fsWatcher, &QFileSystemWatcher::fileChanged, this,
               &Job::fsEvent);

    QStringList watching = _fsWatcher->files() + _fsWatcher->directories();
    if(!watching.isEmpty())
        _fsWatcher->removePaths(watching);
}

QList<ArchivePtr> Job::archives() const
{
    return _archives;
}

void Job::setArchives(const QList<ArchivePtr> &archives)
{
    _archives.clear();
    _archives = archives;
    // Sort archives based on timestamp using an anonymous sorting function.
    std::sort(_archives.begin(), _archives.end(),
              [](const ArchivePtr &a, const ArchivePtr &b) {
                  return (a->timestamp() > b->timestamp());
              });
    // If any Archive is deleted, reload the list.
    for(const ArchivePtr &archive : _archives)
    {
        connect(archive.data(), &Archive::purged, this, &Job::loadArchives,
                Qt::QueuedConnection);
    }
    emit changed();
}

JobSchedule Job::optionScheduledEnabled() const
{
    return static_cast<JobSchedule>(_optionScheduledEnabled);
}

void Job::setOptionScheduledEnabled(JobSchedule schedule)
{
    _optionScheduledEnabled = schedule;
}

bool Job::optionPreservePaths() const
{
    return _optionPreservePaths;
}

void Job::setOptionPreservePaths(bool optionPreservePaths)
{
    _optionPreservePaths = optionPreservePaths;
}

bool Job::optionFollowSymLinks() const
{
    return _optionFollowSymLinks;
}

void Job::setOptionFollowSymLinks(bool optionFollowSymLinks)
{
    _optionFollowSymLinks = optionFollowSymLinks;
}

bool Job::optionTraverseMount() const
{
    return _optionTraverseMount;
}

void Job::setOptionTraverseMount(bool optionTraverseMount)
{
    _optionTraverseMount = optionTraverseMount;
}

int Job::optionSkipFilesSize() const
{
    return _optionSkipFilesSize;
}

void Job::setOptionSkipFilesSize(const int &optionSkipFilesSize)
{
    _optionSkipFilesSize = optionSkipFilesSize;
}

bool Job::optionSkipFiles() const
{
    return _optionSkipFiles;
}

void Job::setOptionSkipFiles(bool optionSkipFiles)
{
    _optionSkipFiles = optionSkipFiles;
}

QString Job::optionSkipFilesPatterns() const
{
    return _optionSkipFilesPatterns;
}

void Job::setOptionSkipFilesPatterns(const QString &optionSkipFilesPatterns)
{
    _optionSkipFilesPatterns = optionSkipFilesPatterns;
}

bool Job::optionSkipNoDump() const
{
    return _optionSkipNoDump;
}

void Job::setOptionSkipNoDump(bool optionSkipNoDump)
{
    _optionSkipNoDump = optionSkipNoDump;
}

bool Job::settingShowHidden() const
{
    return _settingShowHidden;
}

void Job::setSettingShowHidden(bool settingShowHidden)
{
    _settingShowHidden = settingShowHidden;
}

bool Job::settingShowSystem() const
{
    return _settingShowSystem;
}

void Job::setSettingShowSystem(bool settingShowSystem)
{
    _settingShowSystem = settingShowSystem;
}

bool Job::settingHideSymlinks() const
{
    return _settingHideSymlinks;
}

void Job::setSettingHideSymlinks(bool settingHideSymlinks)
{
    _settingHideSymlinks = settingHideSymlinks;
}

void Job::save()
{
    bool exists = doesKeyExist(_name);

    // Prepare query: either updating or creating an entry.
    QString queryString;
    if(exists)
        queryString = QLatin1String(
            "update jobs set name=?, urls=?, optionScheduledEnabled=?,"
            " optionPreservePaths=?, optionTraverseMount=?,"
            " optionFollowSymLinks=?, optionSkipFilesSize=?,"
            " optionSkipFiles=?, optionSkipFilesPatterns=?,"
            " optionSkipNoDump=?, settingShowHidden=?, settingShowSystem=?,"
            " settingHideSymlinks=?"
            " where name=?");
    else
        queryString = QLatin1String(
            "insert into jobs(name, urls, optionScheduledEnabled,"
            " optionPreservePaths, optionTraverseMount,"
            " optionFollowSymLinks, optionSkipFilesSize, optionSkipFiles,"
            " optionSkipFilesPatterns, optionSkipNoDump, settingShowHidden,"
            " settingShowSystem, settingHideSymlinks)"
            " values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    // Get database instance and create query object.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(queryString))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Convert urls to a string.
    QStringList query_urls;
    for(const QUrl &url : _urls)
        query_urls << url.toString(QUrl::FullyEncoded);
    const QString url_str = query_urls.join('\n');

    // Fill in missing values in query string.
    query.addBindValue(_name);
    query.addBindValue(url_str);
    query.addBindValue(_optionScheduledEnabled);
    query.addBindValue(_optionPreservePaths);
    query.addBindValue(_optionTraverseMount);
    query.addBindValue(_optionFollowSymLinks);
    query.addBindValue(_optionSkipFilesSize);
    query.addBindValue(_optionSkipFiles);
    query.addBindValue(_optionSkipFilesPatterns);
    query.addBindValue(_optionSkipNoDump);
    query.addBindValue(_settingShowHidden);
    query.addBindValue(_settingShowSystem);
    query.addBindValue(_settingHideSymlinks);
    if(exists)
        query.addBindValue(_name);

    // Run query.
    if(!global_store->runQuery(query))
        DEBUG << "Failed to save Job entry.";
    setObjectKey(_name);
}

void Job::load()
{
    // Sanity checks.
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to load Job object with empty _name key.";
        return;
    }
    // Get database instance and prepare query.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(QLatin1String("select * from jobs where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Fill in missing value in query string.
    query.addBindValue(_name);
    // Run query.
    if(global_store->runQuery(query) && query.next())
    {
        _urls = QUrl::fromStringList(query.value(query.record().indexOf("urls"))
                                         .toString()
                                         .split('\n', SKIP_EMPTY_PARTS));
        _optionScheduledEnabled =
            query.value(query.record().indexOf("optionScheduledEnabled"))
                .toInt();
        _optionPreservePaths =
            query.value(query.record().indexOf("optionPreservePaths")).toBool();
        _optionTraverseMount =
            query.value(query.record().indexOf("optionTraverseMount")).toBool();
        _optionFollowSymLinks =
            query.value(query.record().indexOf("optionFollowSymLinks"))
                .toBool();
        _optionSkipFilesSize =
            query.value(query.record().indexOf("optionSkipFilesSize")).toInt();
        _optionSkipFiles =
            query.value(query.record().indexOf("optionSkipFiles")).toBool();
        _optionSkipFilesPatterns =
            query.value(query.record().indexOf("optionSkipFilesPatterns"))
                .toString();
        _optionSkipNoDump =
            query.value(query.record().indexOf("optionSkipNoDump")).toBool();
        _settingShowHidden =
            query.value(query.record().indexOf("settingShowHidden")).toBool();
        _settingShowSystem =
            query.value(query.record().indexOf("settingShowSystem")).toBool();
        _settingHideSymlinks =
            query.value(query.record().indexOf("settingHideSymlinks")).toBool();
        setObjectKey(_name);
        emit loadArchives();
    }
    else
    {
        DEBUG << "Job object with key " << _name << " not found.";
    }
}

void Job::purge()
{
    // Sanity checks.
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to delete Job object with empty _name key.";
        return;
    }
    if(!doesKeyExist(_name))
    {
        DEBUG << "No Job object with key " << _name;
        return;
    }
    // Get database instance and prepare query.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(QLatin1String("delete from jobs where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    // Fill in missing value in query string.
    query.addBindValue(_name);
    // Run query.
    if(!global_store->runQuery(query))
        DEBUG << "Failed to remove Job entry.";
    setObjectKey("");
    emit purged();
}

bool Job::doesKeyExist(const QString &key)
{
    // Sanity check.
    if(key.isEmpty())
    {
        DEBUG << "doesKeyExist method called with empty args";
        return false;
    }
    // Get database instance and prepare query.
    QSqlQuery query = global_store->createQuery();
    if(!query.prepare(QLatin1String("select name from jobs where name = ?")))
    {
        DEBUG << query.lastError().text();
        return false;
    }
    // Fill in missing value in query string.
    query.addBindValue(key);
    // Run query.
    if(global_store->runQuery(query))
    {
        if(query.next())
            return true;
    }
    else
    {
        DEBUG << "Failed to run doesKeyExist query for a Job.";
    }
    return false;
}

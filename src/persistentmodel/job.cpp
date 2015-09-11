#include "job.h"
#include "debug.h"

/*TODO: These could be turned into a user setting */
#define JOB_NAME_PREFIX    QLatin1String("Job")
#define JOB_NAME_SEPARATOR QLatin1String("_")

static bool ArchiveCompare (ArchivePtr a, ArchivePtr b) { return (a->timestamp() > b->timestamp()); }

Job::Job(QObject *parent) : QObject(parent), _optionScheduledEnabled(false),
    _optionPreservePaths(true), _optionTraverseMount(true),
    _optionFollowSymLinks(false), _optionSkipFilesSize(0)

{
}

Job::~Job()
{
}

QString Job::name() const
{
    return _name;
}

void Job::setName(const QString &name)
{
    _name = name;
}

QList<QUrl> Job::urls() const
{
    return _urls;
}

void Job::setUrls(const QList<QUrl> &urls)
{
    _urls = urls;
}

QList<ArchivePtr> Job::archives() const
{
    return _archives;
}

void Job::setArchives(const QList<ArchivePtr> &archives)
{
    _archives.clear();
    _archives = archives;
    std::sort(_archives.begin(), _archives.end(), ArchiveCompare);
    foreach (ArchivePtr archive, _archives) {
        connect(archive.data(), SIGNAL(purged()), this, SIGNAL(loadArchives()), Qt::QueuedConnection);
    }
    emit changed();
}

bool Job::optionScheduledEnabled() const
{
    return _optionScheduledEnabled;
}

void Job::setOptionScheduledEnabled(bool optionScheduledEnabled)
{
    _optionScheduledEnabled = optionScheduledEnabled;
}

bool Job::optionPreservePaths() const
{
    return _optionPreservePaths;
}

void Job::setOptionPreservePaths(bool optionPreservePaths)
{
    _optionPreservePaths = optionPreservePaths;
}

quint64 Job::optionSkipFilesSize() const
{
    return _optionSkipFilesSize;
}

void Job::setOptionSkipFilesSize(const quint64 &optionSkipFilesSize)
{
    _optionSkipFilesSize = optionSkipFilesSize;
}

BackupTaskPtr Job::createBackupTask()
{
    BackupTaskPtr backup(new BackupTask);
    backup->setName(JOB_NAME_PREFIX + JOB_NAME_SEPARATOR + name() + JOB_NAME_SEPARATOR + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH:mm:ss"));
    backup->setJobRef(objectKey());
    backup->setUrls(urls());
    backup->setOptionPreservePaths(optionPreservePaths());
    backup->setOptionTraverseMount(optionTraverseMount());
    backup->setOptionFollowSymLinks(optionFollowSymLinks());
    backup->setOptionSkipFilesSize(optionSkipFilesSize());
    connect(backup, SIGNAL(statusUpdate(const TaskStatus&)), this, SLOT(backupTaskUpdate(const TaskStatus&)), Qt::QueuedConnection);
    return backup;
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

void Job::save()
{
    bool exists = findObjectWithKey(_name);

    QString queryString;
    if(exists)
        queryString = QLatin1String("update jobs set name=?, urls=?, optionScheduledEnabled=?, optionPreservePaths=?, "
                                    "optionTraverseMount=?, optionFollowSymLinks=?, optionSkipFilesSize=? "
                                    "where name=?");
    else
        queryString = QLatin1String("insert into jobs(name, urls, optionScheduledEnabled, optionPreservePaths, optionTraverseMount, "
                                    "optionFollowSymLinks, optionSkipFilesSize) values(?, ?, ?, ?, ?, ?, ?)");

    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(queryString))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(_name);
    QStringList urls;
    foreach(QUrl url, _urls)
    {
        urls << url.toString(QUrl::FullyEncoded);
    }
    query.addBindValue(urls.join('\n'));
    query.addBindValue(_optionScheduledEnabled);
    query.addBindValue(_optionPreservePaths);
    query.addBindValue(_optionTraverseMount);
    query.addBindValue(_optionFollowSymLinks);
    query.addBindValue(_optionSkipFilesSize);
    if(exists)
        query.addBindValue(_name);

    store.runQuery(query);
    setObjectKey(_name);
    emit changed();
}

void Job::load()
{
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to load Job object with empty _name key.";
        return;
    }
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select * from jobs where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(_name);
    if(store.runQuery(query) && query.next())
    {
        _urls = QUrl::fromStringList(query.value(query.record().indexOf("urls")).toString().split('\n', QString::SkipEmptyParts));
        _optionScheduledEnabled = query.value(query.record().indexOf("optionScheduledEnabled")).toBool();
        _optionPreservePaths  = query.value(query.record().indexOf("optionPreservePaths")).toBool();
        _optionTraverseMount  = query.value(query.record().indexOf("optionTraverseMount")).toBool();
        _optionFollowSymLinks = query.value(query.record().indexOf("optionFollowSymLinks")).toBool();
        _optionSkipFilesSize  = query.value(query.record().indexOf("optionSkipFilesSize")).toLongLong();
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
    if(_name.isEmpty())
    {
        DEBUG << "Attempting to delete Job object with empty _name key.";
        return;
    }
    if(!findObjectWithKey(_name))
    {
        DEBUG << "No Job object with key " << _name;
        return;
    }
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("delete from jobs where name = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(_name);
    store.runQuery(query);
    setObjectKey("");
}

bool Job::findObjectWithKey(QString key)
{
    bool found = false;
    if(key.isEmpty())
    {
        DEBUG << "findObjectWithKey method called with empty args";
        return found;
    }
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select name from jobs where name = ?")))
    {
        DEBUG << query.lastError().text();
        return found;
    }
    query.addBindValue(key);
    if(store.runQuery(query) && query.next())
    {
        found = true;
    }
    return found;
}

//void Job::loadArchives()
//{
//    if(objectKey().isEmpty())
//    {
//        DEBUG << "loadArchives method called on Job with no objectKey";
//        return;
//    }
//    PersistentStore &store = getStore();
//    QSqlQuery query = store.createQuery();
//    if(!query.prepare(QLatin1String("select * from archives where jobRef = ?")))
//    {
//        DEBUG << query.lastError().text();
//        return;
//    }
//    query.addBindValue(objectKey());
//    if(store.runQuery(query) && query.next())
//    {
//        QList<ArchivePtr> archives;
//        do
//        {
//            ArchivePtr archive(new Archive);
//            archive->setName(query.value(query.record().indexOf("name")).toString());
//            archive->load();
//            if(!archive->objectKey().isEmpty())
//            {
//                connect(archive.data(), SIGNAL(purged()), this, SLOT(loadArchives()), Qt::QueuedConnection);
//                archives << archive;
//            }
//        }while(query.next());
//        setArchives(archives);
//        emit changed();
//    }
//}

void Job::backupTaskUpdate(const TaskStatus& status)
{
    if(status == TaskStatus::Completed)
        loadArchives();
}

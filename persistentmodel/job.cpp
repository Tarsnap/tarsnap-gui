#include "job.h"
#include "debug.h"

Job::Job(QObject *parent) : QObject(parent)
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
    _archives = archives;
}

void Job::save()
{
    bool exists = findObjectWithKey(_name);
    QString queryString;
    if(exists)
        queryString = QLatin1String("update jobs set name=?, urls=? where name=?");
    else
        queryString = QLatin1String("insert into jobs(name, urls)"
                                    " values(?, ?)");
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
    if(exists)
        query.addBindValue(_name);

    QMetaObject::invokeMethod(&getStore(), "runQuery", Qt::QueuedConnection, Q_ARG(QSqlQuery, query));
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
    if(!query.exec())
    {
        DEBUG << query.lastError().text();
        return;
    }
    else if(query.next())
    {
        _urls = QUrl::fromStringList(query.value(query.record().indexOf("urls")).toString().split('\n', QString::SkipEmptyParts));
        setObjectKey(_name);
        loadArchives();
    }
    else
    {
        DEBUG << "Job object with key " << _name << " not found.";
        return;
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
    QMetaObject::invokeMethod(&getStore(), "runQuery", Qt::QueuedConnection, Q_ARG(QSqlQuery, query));
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
    // QMetaObject::invokeMethod(&getStore(), "runQuery", Qt::QueuedConnection, Q_ARG(QSqlQuery, q));
    // we need to get the result here, thus we can't invoke runQuery like that
    // this should be safe nonetheless, since this is a READ only operation
    if(!query.exec())
    {
        DEBUG << query.lastError().text();
        return found;
    }
    else if(query.next())
    {
        found = true;
    }
    return found;
}

void Job::loadArchives()
{
    if(objectKey().isEmpty())
    {
        DEBUG << "loadArchives method called on Job with no objectKey";
        return;
    }
    PersistentStore &store = getStore();
    QSqlQuery query = store.createQuery();
    if(!query.prepare(QLatin1String("select * from archives where jobRef = ?")))
    {
        DEBUG << query.lastError().text();
        return;
    }
    query.addBindValue(objectKey());
    // QMetaObject::invokeMethod(&getStore(), "runQuery", Qt::QueuedConnection, Q_ARG(QSqlQuery, q));
    // we need to get the result here, thus we can't invoke runQuery like that
    // this should be safe nonetheless, since this is a READ only operation
    if(!query.exec())
    {
        DEBUG << query.lastError().text();
        return;
    }
    else if(query.next())
    {
        _archives.clear();
        do
        {
            ArchivePtr archive(new Archive);
            archive->setName(query.value(query.record().indexOf("name")).toString());
            archive->load();
            if(!archive->objectKey().isEmpty())
                _archives << archive;
        }while(query.next());
        emit changed();
    }
}






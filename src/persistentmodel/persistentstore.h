#ifndef PERSISTENTSTORE_H
#define PERSISTENTSTORE_H

#include <QObject>
#include <QtSql>
#include <QMutex>

#define DEFAULT_DBNAME "tarsnap.db"

class PersistentStore : public QObject
{
    Q_OBJECT

public:
    ~PersistentStore();
    static PersistentStore& instance() { static PersistentStore instance;  if(!instance.initialized()) instance.init(); return instance; }
    bool initialized() { return _initialized; }
    QSqlQuery createQuery();
    void purge();
    void lock();
    void unlock();

    QMutex* mutex();

signals:

public slots:
    bool runQuery(QSqlQuery query);

private:
    // Yes, a singleton
    explicit PersistentStore(QObject *parent = 0);
    PersistentStore(PersistentStore const&);
    void operator=(PersistentStore const&);
    bool init();
    void deinit();

    bool            _initialized;
    QSqlDatabase    _db;
    QMutex          _mutex;
};

#endif // PERSISTENTSTORE_H

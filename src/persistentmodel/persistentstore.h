#ifndef PERSISTENTSTORE_H
#define PERSISTENTSTORE_H

#include <QMutex>
#include <QObject>
#include <QtSql>

#define DEFAULT_DBNAME "tarsnap.db"

class PersistentStore : public QObject
{
    Q_OBJECT

public:
    ~PersistentStore();
    static PersistentStore &instance()
    {
        static PersistentStore instance;
        if(!instance.initialized())
            instance.init();
        return instance;
    }
    bool      initialized() { return _initialized; }
    QSqlQuery createQuery();
    void      purge();
    void      lock();
    void      unlock();

public slots:
    bool runQuery(QSqlQuery query);

protected:
    bool upgradeVersion0();
    bool upgradeVersion1();
    bool upgradeVersion2();
    bool upgradeVersion3();
    bool upgradeVersion4();

private:
    // Yes, a singleton
    explicit PersistentStore(QObject *parent = nullptr);
    PersistentStore(PersistentStore const &);
    void operator=(PersistentStore const &);
    bool init();
    void deinit();

    bool         _initialized;
    QSqlDatabase _db;
    QMutex       _mutex;
};

#endif // PERSISTENTSTORE_H

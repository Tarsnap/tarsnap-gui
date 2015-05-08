#ifndef PERSISTENTSTORE_H
#define PERSISTENTSTORE_H

#include <QObject>
#include <QtSql>

#define DEFAULT_DBNAME "tarsnap.db"

class PersistentStore : public QObject
{
    Q_OBJECT
public:
    static PersistentStore& instance() { static PersistentStore instance;  if(!instance.initialized()) instance.init(); return instance; }
    bool initialized() { return _initialized; }
    QSqlQuery createQuery();
    ~PersistentStore();
    void purge();

signals:

public slots:
    void runQuery(QSqlQuery query);

private:
    // Yes, a singleton
    explicit PersistentStore(QObject *parent = 0);
    PersistentStore(PersistentStore const&);
    void operator=(PersistentStore const&);
    bool init();
    void deinit();

    bool            _initialized;
    QSqlDatabase    _db;
};

#endif // PERSISTENTSTORE_H

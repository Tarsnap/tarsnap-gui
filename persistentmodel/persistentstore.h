#ifndef PERSISTENTSTORE_H
#define PERSISTENTSTORE_H

#include <QObject>
#include <QtSql>

#define DEFAULT_DBNAME "tarsnap.db"

class PersistentStore : public QObject
{
    Q_OBJECT
public:
    static void initialize();
    static PersistentStore& instance() { static PersistentStore instance; return instance; }
    static bool initialized() { return _initialized; }
    ~PersistentStore();

signals:

public slots:

private:
    // Yes, a singleton
    explicit PersistentStore(QObject *parent = 0);
    PersistentStore(PersistentStore const&);
    void operator=(PersistentStore const&);

    static bool            _initialized;
    static QSqlDatabase    _db;
};

#endif // PERSISTENTSTORE_H

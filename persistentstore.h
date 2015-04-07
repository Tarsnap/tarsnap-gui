#ifndef PERSISTENTSTORE_H
#define PERSISTENTSTORE_H

#include <QObject>
#include <QtSql>

#define DEFAULT_DBNAME "tarsnap.db"

class PersistentStore : public QObject
{
    Q_OBJECT
public:
    explicit PersistentStore(QObject *parent = 0);
    ~PersistentStore();

signals:

public slots:

private:
    bool _initialized;
    QSqlDatabase _db;
};

#endif // PERSISTENTSTORE_H

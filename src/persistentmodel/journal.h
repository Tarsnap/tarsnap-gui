#ifndef JOURNAL_H
#define JOURNAL_H

#include "persistentobject.h"

#include <QObject>

class Journal : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    explicit Journal(QObject *parent = 0);
    ~Journal();

    // From PersistentObject
    void save(){}
    void load();
    void purge();
    bool findObjectWithKey(QString key){Q_UNUSED(key); return false;}

signals:
    void logEntry(QDateTime timestamp, QString log);
    void journal(QMap<QDateTime, QString> _log);


public slots:
    void getJournal() { emit journal(_log); }
    void log(QString message);

private:
    QMap<QDateTime, QString> _log;
};

#endif // JOURNAL_H

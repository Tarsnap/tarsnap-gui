#ifndef JOURNAL_H
#define JOURNAL_H

#include "persistentobject.h"

#include <QObject>

struct LogEntry
{
    QDateTime timestamp;
    QString   message;
};

class Journal : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    explicit Journal(QObject *parent = nullptr);
    ~Journal();

public slots:
    void getJournal() { emit journal(_log); }
    void log(QString message);

    // From PersistentObject
    void save() {}
    void load();
    void purge();
    bool doesKeyExist(QString key)
    {
        Q_UNUSED(key);
        return false;
    }

signals:
    void logEntry(LogEntry log);
    void journal(QVector<LogEntry> _log);

private:
    QVector<LogEntry> _log;
};

#endif // JOURNAL_H

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

signals:
    void logEntry(LogEntry log);
    void journal(QVector<LogEntry> _log);

public slots:
    void getJournal() { emit journal(_log); }
    void log(QString message);

    // From PersistentObject
    void save() {}
    void load();
    void purge();
    bool findObjectWithKey(QString key)
    {
        Q_UNUSED(key);
        return false;
    }

private:
    QVector<LogEntry> _log;
};

#endif // JOURNAL_H

#ifndef JOURNAL_H
#define JOURNAL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QVector>
WARNINGS_ENABLE

#include "LogEntry.h"

#include "persistentmodel/persistentobject.h"

/*!
 * \ingroup persistent
 * \brief The Journal stores the user's log messages.
 */
class Journal : public PersistentObject
{
    Q_OBJECT

public:
    //! Constructor.
    explicit Journal(QObject *parent = nullptr);
    ~Journal() override;

public slots:
    //! Emits the current log.
    void getJournal() { emit journal(_log); }
    //! Adds a new log message to the journal (and PersistentStore)
    //! after stripping HTML commands from the string.
    void logMessage(const QString &message);

    // From PersistentObject
    //! Does nothing.
    void save() override {}
    //! Loads this object from the PersistentStore.
    void load() override;
    //! Deletes this object from the PersistentStore.
    void purge() override;
    //! Returns whether an object with this key exists in the PersistentStore.
    bool doesKeyExist(const QString &key) override
    {
        Q_UNUSED(key);
        return false;
    }

signals:
    //! A log entry (including time and HTML-stripped message).
    void logEntry(LogEntry log);
    //! The complete set of log entries.
    void journal(QVector<LogEntry> _log);

private:
    QVector<LogEntry> _log;
};

#endif // JOURNAL_H

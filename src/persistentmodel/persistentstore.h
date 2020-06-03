#ifndef PERSISTENTSTORE_H
#define PERSISTENTSTORE_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QSqlQuery>
WARNINGS_ENABLE

/* Set up global PersistentStore. */
class PersistentStore;
extern PersistentStore *global_store;

/*!
 * \ingroup persistent
 * \brief The PersistentStore is a QObject which interfaces with a database
 * which stores lists of archives, jobs, and journal entries.
 */
class PersistentStore : public QObject
{
    Q_OBJECT

public:
    PersistentStore();

    //! Initialize the global PersistentStore object.
    static void initializePersistentStore();

    //! Free the global PersistentStore object.  Can only be
    //! called once per initializePersistentStore().
    static void destroy();

    //! Returns the status of whether the database is initialized or not.
    bool initialized() { return _initialized; }

    //! Returns an empty query attached to the database if it is initialized,
    //! or an unattached query otherwise.
    QSqlQuery createQuery();
    //! Removes the existing database if it is initialized.  Does not lock.
    void purge();

    //! Locks, upgrades the version if it is old, creates a new one otherwise.
    //! Must be run before any other functions in this class.
    bool init();
    //! Closes the database connection (if it exists).  Normally not used by
    //! external classes, with the possible exception of the test suite.
    static void deinit();

public slots:
    //! Locks the database and runs a query.
    bool runQuery(QSqlQuery query);

private:
    static bool _initialized;
};

#endif // PERSISTENTSTORE_H

#ifndef PERSISTENTOBJECT_H
#define PERSISTENTOBJECT_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/*!
 * \ingroup persistent
 * \brief The PersistentObject is an abstract data type for information which
 * is stored in the PersistentStore.
 */
class PersistentObject : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    explicit PersistentObject(QObject *parent = nullptr);

    //! Returns the object key.
    QString objectKey() const;
    //! Sets the object key.
    void setObjectKey(const QString &objectKey);

public slots:
    //! Saves this object to the PersistentStore; creating or
    //! updating as appropriate.
    virtual void save() = 0;
    //! Loads this object from the PersistentStore.
    virtual void load() = 0;
    //! Deletes this object from the PersistentStore.
    virtual void purge() = 0;
    //! Returns whether an object with this key exists in the PersistentStore.
    virtual bool doesKeyExist(const QString &key) = 0;

private:
    QString _objectKey;
};

#endif // PERSISTENTOBJECT_H

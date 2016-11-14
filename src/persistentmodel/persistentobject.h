#ifndef PERSISTENTOBJECT_H
#define PERSISTENTOBJECT_H

#include "persistentstore.h"

/*!
 * \ingroup persistent
 * \brief The PersistentObject is an abstract data type for information which
 * is stored in the PersistentStore.
 */
class PersistentObject
{
public:
    //! Constructor.
    explicit PersistentObject();
    ~PersistentObject();

    //! Saves this object to the PersistentStore; creating or
    //! updating as appropriate.
    virtual void save() = 0;
    //! Loads this object from the PersistentStore.
    virtual void load() = 0;
    //! Deletes this object from the PersistentStore.
    virtual void purge() = 0;
    //! Returns whether an object with this key exists in the PersistentStore.
    virtual bool doesKeyExist(QString key) = 0;

    //! Returns the PersistentStore singleton.
    PersistentStore &getStore();

    //! Returns the object key.
    QString objectKey() const;
    //! Sets the object key.
    void setObjectKey(const QString &objectKey);

private:
    QString _objectKey;
};

#endif // PERSISTENTOBJECT_H

#ifndef PERSISTENTOBJECT_H
#define PERSISTENTOBJECT_H

#include "persistentstore.h"

class PersistentObject
{
public:
    explicit PersistentObject();
    ~PersistentObject();

    virtual void save()  = 0;
    virtual void load()  = 0;
    virtual void purge() = 0;
    virtual bool findObjectWithKey(QString key) = 0;

    PersistentStore& getStore();

private:
    QString          _objectKey;
};

#endif // PERSISTENTOBJECT_H

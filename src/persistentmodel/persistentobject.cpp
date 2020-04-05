#include "persistentobject.h"

#include "debug.h"

PersistentObject::PersistentObject()
{
}

PersistentObject::~PersistentObject()
{
}

QString PersistentObject::objectKey() const
{
    return _objectKey;
}

void PersistentObject::setObjectKey(const QString &objectKey)
{
    _objectKey = objectKey;
}

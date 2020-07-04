#include "persistentobject.h"

PersistentObject::PersistentObject(QObject *parent) : QObject(parent)
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

#include "persistentobject.h"
#include "debug.h"

PersistentObject::PersistentObject()
{

}

PersistentObject::~PersistentObject()
{

}

PersistentStore& PersistentObject::getStore()
{
    PersistentStore& store = PersistentStore::instance();
    if(!store.initialized())
        DEBUG << "PersistentStore was not initialized properly.";
    return store;
}


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
    {
        DEBUG << "PersistentStore was not initialized previously to this call. Please fix.";
        store.initialize(); // this does not guarantee that the initialization will succeed
    }
    return store;
}


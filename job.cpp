#include "job.h"

Job::Job(QObject *parent) : QObject(parent)
{

}

Job::~Job()
{

}
QString Job::name() const
{
    return _name;
}

void Job::setName(const QString &name)
{
    _name = name;
}



#include "joblistitem.h"

JobListItem::JobListItem(JobPtr job) : _job(job)
{
    _ui.setupUi(&_widget);
    _ui.nameLabel->setText(_job->name());
}

JobListItem::~JobListItem()
{

}

QWidget *JobListItem::widget()
{
    return &_widget;
}
JobPtr JobListItem::job() const
{
    return _job;
}

void JobListItem::setJob(const JobPtr &job)
{
    _job = job;
}


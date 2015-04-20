#include "joblistitem.h"

JobListItem::JobListItem(JobPtr job) : _job(job)
{
    _ui.setupUi(&_widget);
    _ui.nameLabel->setText(_job->name());

    _widget.addAction(_ui.actionBackup);
    _widget.addAction(_ui.actionInspect);
    _widget.addAction(_ui.actionRestore);
    _widget.addAction(_ui.actionDelete);
//    _ui.backupButton->setDefaultAction(_ui.actionBackup);
    _ui.inspectButton->setDefaultAction(_ui.actionInspect);
    _ui.restoreButton->setDefaultAction(_ui.actionRestore);
    connect(_ui.actionBackup, SIGNAL(triggered()), this, SIGNAL(requestBackup()), Qt::QueuedConnection);
    connect(_ui.actionInspect, SIGNAL(triggered()), this, SIGNAL(requestInspect()), Qt::QueuedConnection);
    connect(_ui.actionRestore, SIGNAL(triggered()), this, SIGNAL(requestRestore()), Qt::QueuedConnection);
    connect(_ui.actionDelete, SIGNAL(triggered()), this, SIGNAL(requestDelete()), Qt::QueuedConnection);
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


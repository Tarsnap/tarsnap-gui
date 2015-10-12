#include "joblistitem.h"
#include "utils.h"

JobListItem::JobListItem(JobPtr job):
    _useSIPrefixes(false)
{
    QSettings settings;
    _useSIPrefixes = settings.value("app/si_prefixes", false).toBool();

    _ui.setupUi(&_widget);
    _widget.addAction(_ui.actionBackup);
    _widget.addAction(_ui.actionInspect);
    _widget.addAction(_ui.actionRestore);
    _widget.addAction(_ui.actionDelete);
    _ui.inspectButton->setDefaultAction(_ui.actionInspect);
    _ui.restoreButton->setDefaultAction(_ui.actionRestore);
    connect(_ui.backupButton, SIGNAL(clicked()), _ui.actionBackup, SIGNAL(triggered()), QUEUED);
    connect(_ui.actionBackup, SIGNAL(triggered()), this, SIGNAL(requestBackup()), QUEUED);
    connect(_ui.actionInspect, SIGNAL(triggered()), this, SIGNAL(requestInspect()), QUEUED);
    connect(_ui.actionRestore, SIGNAL(triggered()), this, SIGNAL(requestRestore()), QUEUED);
    connect(_ui.actionDelete, SIGNAL(triggered()), this, SIGNAL(requestDelete()), QUEUED);

    setJob(job);
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

    connect(_job.data(), SIGNAL(changed()), this, SLOT(update()), QUEUED);

    _ui.nameLabel->setText(_job->name());
    if(_job->archives().isEmpty())
        _ui.lastBackupLabel->setText(tr("No backup done yet"));
    else
        _ui.lastBackupLabel->setText(_job->archives().first()->timestamp().toString());

    QString detail;
    detail.append(tr("%1 %2 totaling ").arg(_job->archives().count())
                  .arg(_job->archives().count() == 1 ? tr("archive") : tr("archives") ));
    quint64 totalSize = 0;
    foreach(ArchivePtr archive, _job->archives())
    {
        totalSize += archive->sizeTotal();
    }
    detail.append(Utils::humanBytes(totalSize, _useSIPrefixes));

    _ui.detailLabel->setText(detail);
}

void JobListItem::update()
{
    setJob(_job);
}

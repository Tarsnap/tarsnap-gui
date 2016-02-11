#include "joblistitem.h"
#include "utils.h"

JobListItem::JobListItem(JobPtr job)
    : _widget(new QWidget), _useIECPrefixes(false)
{
    QSettings settings;
    _useIECPrefixes = settings.value("app/iec_prefixes", false).toBool();

    _ui.setupUi(_widget);

    connect(_ui.backupButton, &QPushButton::clicked, this,
            &JobListItem::requestBackup);
    connect(_ui.inspectButton, &QToolButton::clicked, this,
            &JobListItem::requestInspect);
    connect(_ui.restoreButton, &QToolButton::clicked, this,
            &JobListItem::requestRestore);

    setJob(job);
}

JobListItem::~JobListItem()
{
}

QWidget *JobListItem::widget()
{
    return _widget;
}
JobPtr JobListItem::job() const
{
    return _job;
}

void JobListItem::setJob(const JobPtr &job)
{
    _job = job;

    connect(_job.data(), &Job::changed, this, &JobListItem::update, QUEUED);

    _ui.nameLabel->setText(_job->name());
    if(_job->archives().isEmpty())
        _ui.lastBackupLabel->setText(tr("No backup done yet"));
    else
        _ui.lastBackupLabel->setText(
            _job->archives().first()->timestamp().toString(Qt::DefaultLocaleLongDate));

    QString detail;
    QString str = _job->archives().count() == 1 ? tr("archive") : tr("archives");
    detail.append(tr("%1 %2 totaling ").arg(_job->archives().count()).arg(str));
    quint64 totalSize = 0;
    foreach(ArchivePtr archive, _job->archives())
    {
        totalSize += archive->sizeTotal();
    }
    detail.append(Utils::humanBytes(totalSize, _useIECPrefixes));

    _ui.detailLabel->setText(detail);
}

void JobListItem::update()
{
    setJob(_job);
}

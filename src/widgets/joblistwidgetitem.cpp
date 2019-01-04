#include "joblistwidgetitem.h"
#include "utils.h"

#include "ui_joblistwidgetitem.h"

JobListWidgetItem::JobListWidgetItem(JobPtr job)
    : _ui(new Ui::JobListWidgetItem), _widget(new QWidget)
{
    _widget->installEventFilter(this);
    _ui->setupUi(_widget);
    updateUi();

    connect(_ui->backupButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestBackup);
    connect(_ui->inspectButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestInspect);
    connect(_ui->restoreButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestRestore);
    connect(_ui->deleteButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestDelete);

    setJob(job);
}

JobListWidgetItem::~JobListWidgetItem()
{
    delete _ui;
}

QWidget *JobListWidgetItem::widget()
{
    return _widget;
}
JobPtr JobListWidgetItem::job() const
{
    return _job;
}

void JobListWidgetItem::setJob(const JobPtr &job)
{
    if(_job)
        disconnect(_job.data(), &Job::changed, this, &JobListWidgetItem::update);

    _job = job;

    connect(_job.data(), &Job::changed, this, &JobListWidgetItem::update,
            QUEUED);

    _ui->nameLabel->setText(_job->name());
    if(_job->archives().isEmpty())
        _ui->lastBackupLabel->setText(tr("No backups"));
    else
    {
        QDateTime timestamp = _job->archives().first()->timestamp();
        _ui->lastBackupLabel->setText(
            timestamp.toString(Qt::DefaultLocaleShortDate));
    }

    QString detail;
    QString str =
        _job->archives().count() == 1 ? tr("archive") : tr("archives");
    detail.append(tr("%1 %2 totaling ").arg(_job->archives().count()).arg(str));
    quint64 totalSize = 0;
    for(const ArchivePtr &archive : _job->archives())
    {
        totalSize += archive->sizeTotal();
    }
    detail.append(Utils::humanBytes(totalSize));

    _ui->detailLabel->setText(detail);
}

void JobListWidgetItem::update()
{
    setJob(_job);
}

bool JobListWidgetItem::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _widget) && (event->type() == QEvent::LanguageChange))
    {
        _ui->retranslateUi(_widget);
        updateUi();
        update();
        return true;
    }
    return false;
}

void JobListWidgetItem::updateUi()
{
    _ui->inspectButton->setToolTip(_ui->inspectButton->toolTip().arg(
        _ui->actionJobInspect->shortcut().toString(QKeySequence::NativeText)));
    _ui->restoreButton->setToolTip(_ui->restoreButton->toolTip().arg(
        _ui->actionJobRestore->shortcut().toString(QKeySequence::NativeText)));
    _ui->backupButton->setToolTip(_ui->backupButton->toolTip().arg(
        _ui->actionJobBackup->shortcut().toString(QKeySequence::NativeText)));
    _ui->deleteButton->setToolTip(_ui->deleteButton->toolTip().arg(
        _ui->actionJobDelete->shortcut().toString(QKeySequence::NativeText)));
}

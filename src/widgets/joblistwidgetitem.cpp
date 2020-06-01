#include "joblistwidgetitem.h"

WARNINGS_DISABLE
#include <QAction>
#include <QDateTime>
#include <QEvent>
#include <QKeySequence>
#include <QSize>
#include <QWidget>
#include <Qt>

#include "ui_joblistwidgetitem.h"
WARNINGS_ENABLE

#include "TElidedLabel.h"

#include "humanbytes.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"

JobListWidgetItem::JobListWidgetItem(JobPtr job)
    : _ui(new Ui::JobListWidgetItem), _widget(new QWidget)
{
    _widget->installEventFilter(this);
    _ui->setupUi(_widget);
    updateKeyboardShortcutInfo();
    // Set a sensible size.
    setSizeHint(QSize(_widget->minimumWidth(), _widget->minimumHeight()));

    // Connections for buttons.
    connect(_ui->backupButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestBackup);
    connect(_ui->inspectButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestInspect);
    connect(_ui->restoreButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestRestore);
    connect(_ui->deleteButton, &QToolButton::clicked, this,
            &JobListWidgetItem::requestDelete);

    // Display info about the Job.
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
    // Disconnect previous Job.
    if(_job)
        disconnect(_job.data(), &Job::changed, this,
                   &JobListWidgetItem::update);

    // Set pointer.
    _job = job;

    // Connection for any modifications: the list of Archives
    // belonging to this Job has been updated.
    connect(_job.data(), &Job::changed, this, &JobListWidgetItem::update,
            Qt::QueuedConnection);

    // Display the Job name.
    _ui->nameLabel->setText(_job->name());

    // Display the datetime of the most recent archive, or "No backups".
    if(_job->archives().isEmpty())
        _ui->lastBackupLabel->setText(tr("No backups"));
    else
    {
        QDateTime timestamp = _job->archives().first()->timestamp();
        _ui->lastBackupLabel->setText(
            timestamp.toString(Qt::DefaultLocaleShortDate));
    }

    // Display the number and total size of Archives.
    updateIEC();
}

void JobListWidgetItem::updateIEC()
{
    int     count  = _job->archives().count();
    QString detail = tr("%1 %2 totaling ")
                         .arg(count)
                         .arg(count == 1 ? tr("archive") : tr("archives"));
    quint64 totalSize = 0;
    for(const ArchivePtr &archive : _job->archives())
    {
        totalSize += archive->sizeTotal();
    }
    detail.append(humanBytes(totalSize));

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
        updateKeyboardShortcutInfo();
        update();
        return true;
    }
    return false;
}

void JobListWidgetItem::updateKeyboardShortcutInfo()
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

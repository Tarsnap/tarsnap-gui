#include "archivelistwidgetitem.h"

WARNINGS_DISABLE
#include <QAction>
#include <QDateTime>
#include <QEvent>
#include <QKeySequence>
#include <QLatin1String>
#include <QSize>
#include <QToolButton>
#include <QVector>
#include <QWidget>
#include <Qt>

#include "ui_archivelistwidgetitem.h"
WARNINGS_ENABLE

#include "TElidedLabel.h"

#include "humanbytes.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "widgets/elidedannotatedlabel.h"

#define FIELD_WIDTH 6

ArchiveListWidgetItem::ArchiveListWidgetItem(ArchivePtr archive)
    : _ui(new Ui::ArchiveListWidgetItem), _widget(new QWidget)
{
    _ui->setupUi(_widget);
    // Send translation events to the widget.
    _widget->installEventFilter(this);
    updateKeyboardShortcutInfo();
    // Set a sensible size.
    setSizeHint(QSize(_widget->minimumWidth(), _widget->minimumHeight()));

    // Set up action connections.
    connect(_ui->deleteButton, &QToolButton::clicked, this,
            &ArchiveListWidgetItem::requestDelete);
    connect(_ui->inspectButton, &QToolButton::clicked, this,
            &ArchiveListWidgetItem::requestInspect);
    connect(_ui->restoreButton, &QToolButton::clicked, this,
            &ArchiveListWidgetItem::requestRestore);
    // Load the archive.
    setArchive(archive);
}

ArchiveListWidgetItem::~ArchiveListWidgetItem()
{
    delete _ui;
}

QWidget *ArchiveListWidgetItem::widget()
{
    return _widget;
}

ArchivePtr ArchiveListWidgetItem::archive() const
{
    return _archive;
}

void ArchiveListWidgetItem::setShowingDetails(bool is_showing)
{
    _ui->inspectButton->setChecked(is_showing);
}

void ArchiveListWidgetItem::setArchive(ArchivePtr archive)
{
    // Remove previous archive.
    if(_archive)
    {
        disconnect(_archive.data(), &Archive::changed, this,
                   &ArchiveListWidgetItem::updateStatus);
        disconnect(_archive.data(), &Archive::purged, this,
                   &ArchiveListWidgetItem::removeItem);
    }

    // Set pointer.
    _archive = archive;

    // Connections for any modifications: being scheduled for deletion,
    // and being scheduled to be saved (i.e. the initial upload).
    connect(_archive.data(), &Archive::changed, this,
            &ArchiveListWidgetItem::updateStatus, Qt::QueuedConnection);
    connect(_archive.data(), &Archive::purged, this,
            &ArchiveListWidgetItem::removeItem, Qt::QueuedConnection);

    // For non-Job Archives, the name is displayed in black.
    // For Archives that were created due to a Job, the name is
    // displayed in three parts, which are coloured grey_black_grey:
    //     Job_JOBNAME_DATE
    // For example,
    //     Job_documents_2020-04-21_14-35-59
    QVector<QString> texts(3, "");
    QVector<QString> annotations(6, "");

    // Display the Job_ prefix (if applicable).
    QString baseName = _archive->name();
    if(!_archive->jobRef().isEmpty()
       && _archive->name().startsWith(JOB_NAME_PREFIX))
    {
        texts[0]       = JOB_NAME_PREFIX;
        annotations[0] = "<font color=\"grey\">";
        annotations[1] = "</font>";
        baseName.remove(0, JOB_NAME_PREFIX.size());
    }
    // Display the date suffix (if applicable).
    if(baseName.size() > ARCHIVE_TIMESTAMP_FORMAT.size())
    {
        QString truncated;
        if(baseName.endsWith(QLatin1String(".part")))
        {
            truncated = QLatin1String(".part");
            baseName.chop(truncated.size());
        }
        QString   timestamp = baseName.right(ARCHIVE_TIMESTAMP_FORMAT.size());
        QDateTime validate =
            QDateTime::fromString(timestamp, ARCHIVE_TIMESTAMP_FORMAT);
        if(validate.isValid())
        {
            baseName.chop(timestamp.size());
            texts[2]       = QString("%1%2").arg(timestamp).arg(truncated);
            annotations[4] = "<font color=\"grey\">";
            annotations[5] = "</font>";
        }
    }
    // Display the archive "base" name (i.e. without "Job_" or the date).
    texts[1] = baseName;
    _ui->nameLabel->setAnnotatedText(texts, annotations);
    _ui->nameLabel->setToolTip(_archive->name());

    // Display a message about upcoming deletion (if applicable),
    // or else the date & size.
    updateStatus();

    // Display the Archive stats as a tooltip.
    _ui->detailLabel->setToolTip(_archive->archiveStats());
}

void ArchiveListWidgetItem::updateStatus()
{
    // Display a message about upcoming deletion (if applicable),
    // or else the date & size.
    if(_archive->deleteScheduled())
    {
        _ui->detailLabel->setText(tr("(scheduled for deletion)"));
        _widget->setEnabled(false);
    }
    else
    {
        // Prepare to display the date (in a separate field, shown for
        // all Archives, not only the Job-related Archives).
        QString detail(
            _archive->timestamp().toString(Qt::DefaultLocaleShortDate));

        // Prepare to display the size.
        if(_archive->sizeTotal() != 0)
        {
            QString size = humanBytes(_archive->sizeTotal(), FIELD_WIDTH);
            detail.prepend(size + "  ");
        }

        // Display date & size.
        _ui->detailLabel->setText(detail);
        _widget->setEnabled(true);
    }
}

void ArchiveListWidgetItem::updateIEC()
{
    // Since the size is only part of the detail label, it's easiest
    // to run the whole updateStatus() again.
    updateStatus();
}

bool ArchiveListWidgetItem::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _widget) && (event->type() == QEvent::LanguageChange))
    {
        _ui->retranslateUi(_widget);
        updateKeyboardShortcutInfo();
        updateStatus();
        return true;
    }
    return false;
}

void ArchiveListWidgetItem::updateKeyboardShortcutInfo()
{
    // Display tooltips using platform-specific strings.
    _ui->inspectButton->setToolTip(_ui->inspectButton->toolTip().arg(
        _ui->actionInspect->shortcut().toString(QKeySequence::NativeText)));
    _ui->restoreButton->setToolTip(_ui->restoreButton->toolTip().arg(
        _ui->actionRestore->shortcut().toString(QKeySequence::NativeText)));
    _ui->deleteButton->setToolTip(_ui->deleteButton->toolTip().arg(
        _ui->actionDelete->shortcut().toString(QKeySequence::NativeText)));
}

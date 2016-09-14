#include "archivelistitem.h"
#include "utils.h"

#define FIELD_WIDTH 6

ArchiveListItem::ArchiveListItem(ArchivePtr archive)
    : _widget(new QWidget)
{
    _widget->installEventFilter(this);
    _ui.setupUi(_widget);

    _ui.inspectButton->setToolTip(_ui.inspectButton->toolTip()
                                   .arg(_ui.actionInspect->shortcut()
                                        .toString(QKeySequence::NativeText)));
    _ui.restoreButton->setToolTip(_ui.restoreButton->toolTip()
                                   .arg(_ui.actionRestore->shortcut()
                                        .toString(QKeySequence::NativeText)));
    _ui.deleteButton->setToolTip(_ui.deleteButton->toolTip()
                                   .arg(_ui.actionDelete->shortcut()
                                        .toString(QKeySequence::NativeText)));

    connect(_ui.deleteButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestDelete);
    connect(_ui.inspectButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestInspect);
    connect(_ui.restoreButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestRestore);
    connect(_ui.jobButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestGoToJob);
    connect(_ui.archiveButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestInspect);

    setArchive(archive);
}

ArchiveListItem::~ArchiveListItem()
{
}

QWidget *ArchiveListItem::widget()
{
    return _widget;
}

ArchivePtr ArchiveListItem::archive() const
{
    return _archive;
}

void ArchiveListItem::setArchive(ArchivePtr archive)
{
    _archive = archive;

    connect(_archive.data(), &Archive::changed, this, &ArchiveListItem::update,
            QUEUED);

    QString displayName;
    QString baseName = _archive->name();
    if(!_archive->jobRef().isEmpty()
       && _archive->name().startsWith(JOB_NAME_PREFIX))
    {
        displayName = QString("<font color=\"grey\">%1</font>").arg(JOB_NAME_PREFIX);
        baseName.remove(0, JOB_NAME_PREFIX.size());
    }
    if(baseName.size() > ARCHIVE_TIMESTAMP_FORMAT.size())
    {
        QString timestamp = baseName.right(ARCHIVE_TIMESTAMP_FORMAT.size());
        QDateTime validate = QDateTime::fromString(timestamp, ARCHIVE_TIMESTAMP_FORMAT);
        if(validate.isValid())
        {
            baseName.chop(timestamp.size());
            baseName += QString("<font color=\"grey\">%1</font>").arg(timestamp);
        }
    }
    displayName += baseName;
    _ui.nameLabel->setText(displayName);
    _ui.nameLabel->setToolTip(_archive->name());
    QString detail(_archive->timestamp().toString(Qt::DefaultLocaleLongDate));
    if(_archive->sizeTotal() != 0)
    {
        QString size = Utils::humanBytes(_archive->sizeTotal(), FIELD_WIDTH);
        detail.prepend(size + "  ");
    }
    _ui.detailLabel->setText(detail);
    _ui.detailLabel->setToolTip(_archive->archiveStats());

    if(_archive->jobRef().isEmpty())
    {
        _ui.jobButton->hide();
        _ui.archiveButton->show();
    }
    else
    {
        _ui.archiveButton->hide();
        _ui.jobButton->show();
    }
}

void ArchiveListItem::setDisabled()
{
    _ui.detailLabel->setText(tr("(scheduled for deletion)"));
    widget()->setEnabled(false);
}

bool ArchiveListItem::isDisabled()
{
    return !widget()->isEnabled();
}

void ArchiveListItem::update()
{
    setArchive(_archive);
}

bool ArchiveListItem::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _widget) && (event->type() == QEvent::LanguageChange))
    {
        _ui.retranslateUi(_widget);
        return true;
    }
    return false;
}

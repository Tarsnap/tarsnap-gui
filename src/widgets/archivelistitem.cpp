#include "archivelistitem.h"
#include "utils.h"

#include <QSettings>

#define FIELD_WIDTH 6

ArchiveListItem::ArchiveListItem(ArchivePtr archive)
    : _widget(new QWidget), _useIECPrefixes(false)
{
    QSettings settings;
    _useIECPrefixes = settings.value("app/iec_prefixes", false).toBool();

    _ui.setupUi(_widget);

    connect(_ui.deleteButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestDelete);
    connect(_ui.inspectButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestInspect);
    connect(_ui.restoreButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestRestore);
    connect(_ui.jobButton, &QToolButton::clicked, this,
            &ArchiveListItem::requestGoToJob);

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

    _ui.nameLabel->setText(_archive->name());
    _ui.nameLabel->setToolTip(_archive->name());
    QString detail(_archive->timestamp().toString(Qt::DefaultLocaleLongDate));
    if(_archive->sizeTotal() != 0)
    {
        QString size = Utils::humanBytes(_archive->sizeTotal(), _useIECPrefixes,
                                         FIELD_WIDTH);
        detail.prepend(size + "  ");
    }
    _ui.detailLabel->setText(detail);
    _ui.detailLabel->setToolTip(_archive->archiveStats());

    if(_archive->jobRef().isEmpty())
    {
        _ui.jobButton->hide();
        _ui.horizontalLayout->removeWidget(_ui.jobButton);
        _ui.archiveButton->show();
    }
    else
    {
        _ui.archiveButton->hide();
        _ui.horizontalLayout->removeWidget(_ui.archiveButton);
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

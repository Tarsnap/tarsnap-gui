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
    _widget->addAction(_ui.actionInspect);
    _widget->addAction(_ui.actionRestore);
    _widget->addAction(_ui.actionDelete);
    _ui.inspectButton->setDefaultAction(_ui.actionInspect);
    _ui.jobButton->setDefaultAction(_ui.actionGoToJob);
    _ui.restoreButton->setDefaultAction(_ui.actionRestore);
    _ui.deleteButton->setDefaultAction(_ui.actionDelete);
    connect(_ui.actionDelete, &QAction::triggered, this,
            &ArchiveListItem::requestDelete);
    connect(_ui.actionInspect, &QAction::triggered, this,
            &ArchiveListItem::requestInspect);
    connect(_ui.actionRestore, &QAction::triggered, this,
            &ArchiveListItem::requestRestore);
    connect(_ui.actionGoToJob, &QAction::triggered, this,
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
        _widget->removeAction(_ui.actionGoToJob);
    }
    else
    {
        _ui.archiveButton->hide();
        _ui.horizontalLayout->removeWidget(_ui.archiveButton);
        _ui.jobButton->show();
        _widget->insertAction(_ui.actionRestore, _ui.actionGoToJob);
    }
}

void ArchiveListItem::update()
{
    setArchive(_archive);
}

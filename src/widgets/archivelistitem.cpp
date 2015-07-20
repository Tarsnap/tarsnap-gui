#include "archivelistitem.h"
#include "utils.h"

#include <QSettings>

ArchiveListItem::ArchiveListItem(ArchivePtr archive):
    _useSIPrefixes(false)
{
    QSettings settings;
    _useSIPrefixes = settings.value("app/si_prefixes", false).toBool();

    _ui.setupUi(&_widget);
    _widget.addAction(_ui.actionInspect);
    _widget.addAction(_ui.actionRestore);
    _widget.addAction(_ui.actionDelete);
    _ui.inspectButton->setDefaultAction(_ui.actionInspect);
    _ui.jobButton->setDefaultAction(_ui.actionGoToJob);
    _ui.restoreButton->setDefaultAction(_ui.actionRestore);
    _ui.deleteButton->setDefaultAction(_ui.actionDelete);
    connect(_ui.actionDelete, SIGNAL(triggered()), this, SIGNAL(requestDelete()), Qt::QueuedConnection);
    connect(_ui.actionInspect, SIGNAL(triggered()), this, SIGNAL(requestInspect()), Qt::QueuedConnection);
    connect(_ui.actionRestore, SIGNAL(triggered()), this, SIGNAL(requestRestore()), Qt::QueuedConnection);
    connect(_ui.actionGoToJob, SIGNAL(triggered()), this, SIGNAL(requestGoToJob()), Qt::QueuedConnection);

    setArchive(archive);
}

ArchiveListItem::~ArchiveListItem()
{
}

QWidget *ArchiveListItem::widget()
{
    return &_widget;
}

ArchivePtr ArchiveListItem::archive() const
{
    return _archive;
}

void ArchiveListItem::setArchive(ArchivePtr archive)
{
    _archive = archive;

    connect(_archive.data(), SIGNAL(changed()), this, SLOT(update()), Qt::QueuedConnection);

    _ui.nameLabel->setText(_archive->name());
    _ui.nameLabel->setToolTip(_archive->name());
    QString detail(_archive->timestamp().toString());
    if(_archive->sizeTotal() != 0)
    {
        detail.prepend(Utils::humanBytes(_archive->sizeTotal(), _useSIPrefixes) + "  ");
    }
    _ui.detailLabel->setText(detail);
    _ui.detailLabel->setToolTip(_archive->archiveStats());

    if(_archive->jobRef().isEmpty())
    {
        _ui.jobButton->hide();
        _ui.horizontalLayout->removeWidget(_ui.jobButton);
        _ui.iconLabel->show();
        _widget.removeAction(_ui.actionGoToJob);
    }
    else
    {
        _ui.iconLabel->hide();
        _ui.horizontalLayout->removeWidget(_ui.iconLabel);
        _ui.jobButton->show();
        _widget.insertAction(_ui.actionRestore, _ui.actionGoToJob);
    }
}

void ArchiveListItem::update()
{
    setArchive(_archive);
}




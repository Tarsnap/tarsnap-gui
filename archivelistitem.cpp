#include "archivelistitem.h"

ArchiveListItem::ArchiveListItem(QSharedPointer<Archive> archive, QObject *parent):
    QObject(parent)
{
    _ui.setupUi(&_widget);
    connect(_ui.deleteButton, SIGNAL(clicked()), this, SIGNAL(requestDelete()));
    connect(_ui.inspectButton, SIGNAL(clicked()), this, SIGNAL(requestInspect()));
    connect(_ui.restoreButton, SIGNAL(clicked()), this, SIGNAL(requestRestore()));
    setArchive(archive);
}

ArchiveListItem::~ArchiveListItem()
{

}

QWidget *ArchiveListItem::widget()
{
    return &_widget;
}
QSharedPointer<Archive> ArchiveListItem::archive() const
{
    return _archive;
}

void ArchiveListItem::setArchive(QSharedPointer<Archive> archive)
{
    _archive = archive;

    _ui.nameLabel->setText(_archive->name);
    QString detail(_archive->timestamp.toString());
    if(archive->sizeTotal != 0)
    {
        detail.prepend(tr("%1 bytes  ").arg(_archive->sizeUniqueCompressed));
    }
    _ui.detaiLabel->setText(detail);
}




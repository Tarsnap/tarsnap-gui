#include "archivelistitem.h"

ArchiveListItem::ArchiveListItem(ArchivePtr archive, QObject *parent):
    QObject(parent)
{
    _widget.installEventFilter(this);
    _ui.setupUi(&_widget);
    _widget.addAction(_ui.actionDelete);
    _widget.addAction(_ui.actionInspect);
    _widget.addAction(_ui.actionRestore);
    _ui.deleteButton->setDefaultAction(_ui.actionDelete);
    _ui.inspectButton->setDefaultAction(_ui.actionInspect);
    _ui.restoreButton->setDefaultAction(_ui.actionRestore);
    connect(_ui.actionDelete, SIGNAL(triggered()), this, SIGNAL(requestDelete()), Qt::QueuedConnection);
    connect(_ui.actionInspect, SIGNAL(triggered()), this, SIGNAL(requestInspect()), Qt::QueuedConnection);
    connect(_ui.actionRestore, SIGNAL(triggered()), this, SIGNAL(requestRestore()), Qt::QueuedConnection);

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

    _ui.nameLabel->setText(_archive->name);
    QString detail(_archive->timestamp.toString());
    if(archive->sizeUniqueCompressed != 0)
    {
        detail.prepend(tr("%1 bytes  ").arg(_archive->sizeUniqueCompressed));
    }
    _ui.detaiLabel->setText(detail);
}

void ArchiveListItem::update()
{
    setArchive(_archive);
}

bool ArchiveListItem::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        emit requestInspect();
        return true;
    } else {
        // standard event processing
        return _widget.eventFilter(obj, event);
    }
}




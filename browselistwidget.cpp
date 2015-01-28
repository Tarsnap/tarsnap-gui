#include "browselistwidget.h"
#include "archivelistitem.h"
#include "ui_archiveitemwidget.h"

#include <QDebug>
#include <QMessageBox>

BrowseListWidget::BrowseListWidget(QWidget *parent):
    QListWidget(parent)
{
}

BrowseListWidget::~BrowseListWidget()
{
    clear();
}

void BrowseListWidget::addArchives(QList<ArchivePtr > archives)
{
    clear();
    foreach (ArchivePtr archive, archives) {
        ArchiveListItem *item = new ArchiveListItem(archive);
        connect(item, SIGNAL(requestDelete()), this, SLOT(removeItem()));
        connect(item, SIGNAL(requestInspect()), this, SLOT(inspectItem()));
        connect(item, SIGNAL(requestRestore()), this, SLOT(restoreItem()));
        this->insertItem(this->count(), item);
        this->setItemWidget(item, item->widget());
    }
}

void BrowseListWidget::removeItem()
{
    ArchiveListItem* archiveItem = qobject_cast<ArchiveListItem*>(sender());
    if(archiveItem)
    {
        emit deleteArchive(archiveItem->archive());
        QListWidgetItem *item = this->takeItem(this->row(archiveItem));
        if(item)
            delete item;
    }
}

void BrowseListWidget::inspectItem()
{
    ArchiveListItem* archiveItem = qobject_cast<ArchiveListItem*>(sender());
    if(archiveItem)
    {
        emit inspectArchive(archiveItem->archive());
    }
}

void BrowseListWidget::restoreItem()
{

}

void BrowseListWidget::removeSelectedItems()
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Confirm action")
                                                               , tr("Are you sure you want to remove %1 selected archives?").arg(this->selectedItems().count()));
    if(button == QMessageBox::Yes)
    {
        foreach (QListWidgetItem *item, this->selectedItems())
        {
            if(item->isSelected())
            {
                QListWidgetItem *takenItem = this->takeItem(this->row(item));
                if(takenItem)
                {
                    ArchiveListItem* archiveItem = dynamic_cast<ArchiveListItem*>(takenItem);
                    emit deleteArchive(archiveItem->archive());
                    delete takenItem;
                }
            }
        }
    }
}

void BrowseListWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        removeSelectedItems();
        break;
    case Qt::Key_Escape:
        clearSelection();
        break;
    default:
        QListWidget::keyReleaseEvent(event);
    }
}


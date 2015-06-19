#include "archivelistwidget.h"
#include "archivelistitem.h"
#include "ui_archiveitemwidget.h"
#include "restoredialog.h"

#include <QMessageBox>

static bool ArchiveCompare (ArchivePtr a, ArchivePtr b) { return (a->timestamp() > b->timestamp()); }

ArchiveListWidget::ArchiveListWidget(QWidget *parent):
    QListWidget(parent)
{
    connect(this, &QListWidget::itemActivated,
            [=](QListWidgetItem* item)
            {
                if(item)
                {
                    emit inspectArchive(static_cast<ArchiveListItem*>(item)->archive());
                }
            });
}

ArchiveListWidget::~ArchiveListWidget()
{
    clear();
}

void ArchiveListWidget::addArchives(QList<ArchivePtr > archives)
{
    clear();
    std::sort(archives.begin(), archives.end(), ArchiveCompare);
    foreach (ArchivePtr archive, archives)
    {
        ArchiveListItem *item = new ArchiveListItem(archive);
        connect(item, SIGNAL(requestDelete()), this, SLOT(removeItems()));
        connect(item, SIGNAL(requestInspect()), this, SLOT(inspectItem()));
        connect(item, SIGNAL(requestRestore()), this, SLOT(restoreItem()));
        this->insertItem(this->count(), item);
        this->setItemWidget(item, item->widget());
    }
}

void ArchiveListWidget::removeItems()
{
    if(this->selectedItems().isEmpty())
    {
        // attempt to remove the sender
        ArchiveListItem* archiveItem = qobject_cast<ArchiveListItem*>(sender());
        if(archiveItem)
        {
            ArchivePtr archive = archiveItem->archive();
            QMessageBox::StandardButton button = QMessageBox::question(this, tr("Confirm action")
                                                                       , tr("Are you sure you want to delete archive %1 (this cannot be undone)?").arg(archive->name()));
            if(button == QMessageBox::Yes)
            {
                QList<ArchivePtr> archiveList;
                archiveList.append(archive);
                emit deleteArchives(archiveList);
                // TODO: maybe delete after backend confirmation
                QListWidgetItem *item = this->takeItem(this->row(archiveItem));
                if(item) delete item;
            }
        }
    }
    else
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Confirm action")
                                                                   , tr("Are you sure you want to delete %1 selected archives (this cannot be undone)?").arg(this->selectedItems().count()));
        if(button == QMessageBox::Yes)
        {
            QList<ArchivePtr> archiveList;
            foreach (QListWidgetItem *item, this->selectedItems())
            {
                if(item->isSelected())
                {
                    // TODO: maybe delete after backend confirmation
                    QListWidgetItem *takenItem = this->takeItem(this->row(item));
                    if(takenItem)
                    {
                        ArchiveListItem* archiveItem = static_cast<ArchiveListItem*>(takenItem);
                        archiveList.append(archiveItem->archive());
                        delete archiveItem;
                    }
                }
            }
            if(!archiveList.isEmpty())
                emit deleteArchives(archiveList);
        }
    }
}

void ArchiveListWidget::inspectItem()
{
    if(sender())
    {
        emit inspectArchive(qobject_cast<ArchiveListItem*>(sender())->archive());
    }
}

void ArchiveListWidget::restoreItem()
{
    ArchiveListItem* archiveItem = qobject_cast<ArchiveListItem*>(sender());
    if(archiveItem)
    {
        RestoreDialog restoreDialog(archiveItem->archive(), this);
        if( QDialog::Accepted == restoreDialog.exec())
            emit restoreArchive(archiveItem->archive(), restoreDialog.getOptions());
    }
}

void ArchiveListWidget::setSelectedArchive(ArchivePtr archive)
{
    if(!archive)
        return;

    ArchiveListItem* archiveItem = static_cast<ArchiveListItem*>(this->currentItem());
    if(!archiveItem || (archiveItem->archive() != archive))
    {
        for(int i = 0; i < this->count(); ++i)
        {
            ArchiveListItem* archiveItem = static_cast<ArchiveListItem*>(this->item(i));
            if(archiveItem && (archiveItem->archive()->objectKey() == archive->objectKey()))
            {
                this->setCurrentItem(archiveItem);
            }
        }
    }
}

void ArchiveListWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        removeItems();
        break;
    case Qt::Key_Escape:
        if(!selectedItems().isEmpty())
            clearSelection();
        else
            QListWidget::keyReleaseEvent(event);
        break;
    default:
        QListWidget::keyReleaseEvent(event);
    }
}


#include "browselistwidget.h"
#include "archivelistitem.h"
#include "ui_archiveitemwidget.h"
#include "restoredialog.h"

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
        connect(item, SIGNAL(requestDelete()), this, SLOT(removeItems()));
        connect(item, SIGNAL(requestInspect()), this, SLOT(inspectItem()));
        connect(item, SIGNAL(requestRestore()), this, SLOT(restoreItem()));
        this->insertItem(this->count(), item);
        this->setItemWidget(item, item->widget());
    }
}

void BrowseListWidget::removeItems()
{
    if(this->selectedItems().count() == 0)
    {
        // attempt to remove the sender
        ArchiveListItem* archiveItem = qobject_cast<ArchiveListItem*>(sender());
        if(archiveItem)
        {
            QMessageBox::StandardButton button = QMessageBox::question(this, tr("Confirm action")
                                                                       , tr("Are you sure you want to delete %1 (this cannot be undone)?").arg(archiveItem->archive()->name));
            if(button == QMessageBox::Yes)
            {
                QList<ArchivePtr> archiveList;
                archiveList.append(archiveItem->archive());
                emit deleteArchives(archiveList);
                QListWidgetItem *item = this->takeItem(this->row(archiveItem));
                if(item)
                    delete item;
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
                    QListWidgetItem *takenItem = this->takeItem(this->row(item));
                    if(takenItem)
                    {
                        ArchiveListItem* archiveItem = dynamic_cast<ArchiveListItem*>(takenItem);
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
    ArchiveListItem* archiveItem = qobject_cast<ArchiveListItem*>(sender());
    if(archiveItem)
    {
        RestoreDialog restoreDialog(archiveItem->archive(), this);
        if( QDialog::Accepted == restoreDialog.exec())
            emit restoreArchive(archiveItem->archive(), restoreDialog.getOptions());
    }
}

void BrowseListWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        removeItems();
        break;
    case Qt::Key_Escape:
        if(this->selectedItems().count() != 0)
            clearSelection();
        else
            QListWidget::keyReleaseEvent(event);
        break;
    default:
        QListWidget::keyReleaseEvent(event);
    }
}


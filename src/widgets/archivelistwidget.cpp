#include "archivelistwidget.h"
#include "archivelistwidgetitem.h"
#include "restoredialog.h"

#include <QMessageBox>
#include <QKeyEvent>

#define DELETE_CONFIRMATION_THRESHOLD 10

ArchiveListWidget::ArchiveListWidget(QWidget *parent) : QListWidget(parent)
{
    connect(this, &QListWidget::itemActivated, [&](QListWidgetItem *item) {
        if(item)
        {
            ArchiveListWidgetItem *archiveItem = static_cast<ArchiveListWidgetItem *>(item);
            if(archiveItem && !archiveItem->isDisabled())
                emit inspectArchive(archiveItem->archive());
        }
    });
}

ArchiveListWidget::~ArchiveListWidget()
{
    clear();
}

void ArchiveListWidget::addArchives(QList<ArchivePtr> archives)
{
    std::sort(archives.begin(), archives.end(),
              [](const ArchivePtr &a, const ArchivePtr &b) {
                  return (a->timestamp() > b->timestamp());
              });
    setUpdatesEnabled(false);
    clear();
    foreach(ArchivePtr archive, archives)
    {
        ArchiveListWidgetItem *item = new ArchiveListWidgetItem(archive);
        connect(item, &ArchiveListWidgetItem::requestDelete, this,
                &ArchiveListWidget::removeItem);
        connect(item, &ArchiveListWidgetItem::requestInspect, this,
                &ArchiveListWidget::inspectItem);
        connect(item, &ArchiveListWidgetItem::requestRestore, this,
                &ArchiveListWidget::restoreItem);
        connect(item, &ArchiveListWidgetItem::requestGoToJob, this,
                &ArchiveListWidget::goToJob);
        insertItem(count(), item);
        setItemWidget(item, item->widget());
    }
    setUpdatesEnabled(true);
}

void ArchiveListWidget::removeItem()
{
    ArchiveListWidgetItem *archiveItem = qobject_cast<ArchiveListWidgetItem *>(sender());
    if(archiveItem)
    {
        ArchivePtr archive = archiveItem->archive();
        auto       button =
            QMessageBox::question(this, tr("Confirm delete"),
                                  tr("Are you sure you want to delete "
                                     "archive %1 (this cannot be undone)?")
                                      .arg(archive->name()));
        if(button == QMessageBox::Yes)
        {
            archiveItem->setDisabled();
            QList<ArchivePtr> archiveList;
            archiveList.append(archive);
            emit deleteArchives(archiveList);
        }
    }
}

void ArchiveListWidget::removeSelectedItems()
{
    if(selectedItems().isEmpty())
        return;

    QList<ArchiveListWidgetItem *> selectedListItems;
    // Any archives pending deletion in the selection? if so deny action
    foreach(QListWidgetItem *item, selectedItems())
    {
        ArchiveListWidgetItem *archiveItem = static_cast<ArchiveListWidgetItem *>(item);
        if(!archiveItem || archiveItem->isDisabled())
            return;
        else
            selectedListItems << archiveItem;
    }

    int  selectedItemsCount = selectedItems().count();
    auto button             = QMessageBox::question(this, tr("Confirm delete"),
                                        tr("Are you sure you want to "
                                           "delete %1 selected archive(s) "
                                           "(this cannot be undone)?")
                                            .arg(selectedItemsCount));
    if(button == QMessageBox::Yes)
    {
        // Some more deletion confirmation, if count of archives to be
        // removed is bigger than threshold
        if(selectedItemsCount >= DELETE_CONFIRMATION_THRESHOLD)
        {
            // Inform of purge operation if all archives are to be removed
            if(selectedItemsCount == count())
            {
                button = QMessageBox::question(
                    this, tr("Confirm delete"),
                    tr("Are you sure you want to delete all of your "
                       "archives?\n"
                       "For your information, there's a purge action in "
                       "Settings -> Advanced page that achieves the same "
                       "thing but more efficiently."));
            }
            else
            {
                button = QMessageBox::question(this, tr("Confirm delete"),
                                               tr("This will permanently "
                                                  "delete the %1 selected "
                                                  "archives. Proceed?")
                                                   .arg(selectedItemsCount));
            }
        }
    }

    if(button == QMessageBox::Yes)
    {
        QList<ArchivePtr> archivesToDelete;
        foreach(ArchiveListWidgetItem *archiveItem, selectedListItems)
        {
            archiveItem->setDisabled();
            archivesToDelete.append(archiveItem->archive());
        }
        if(!archivesToDelete.isEmpty())
            emit deleteArchives(archivesToDelete);
    }
}

void ArchiveListWidget::inspectSelectedItem()
{
    if(!selectedItems().isEmpty())
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(selectedItems().first());
        if(archiveItem && !archiveItem->isDisabled())
            emit inspectArchive(archiveItem->archive());
    }
}

void ArchiveListWidget::restoreSelectedItem()
{
    if(!selectedItems().isEmpty())
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(selectedItems().first());
        if(archiveItem && !archiveItem->isDisabled())
        {
            RestoreDialog restoreDialog(archiveItem->archive(), this);
            if(QDialog::Accepted == restoreDialog.exec())
                emit restoreArchive(archiveItem->archive(),
                                    restoreDialog.getOptions());
        }
    }
}

void ArchiveListWidget::setFilter(QString regex)
{
    clearSelection();
    QRegExp rx(regex, Qt::CaseInsensitive, QRegExp::Wildcard);
    for(int i = 0; i < count(); ++i)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(i));
        if(archiveItem)
        {
            if(archiveItem->archive()->name().contains(rx))
                archiveItem->setHidden(false);
            else
                archiveItem->setHidden(true);
        }
    }
}

void ArchiveListWidget::inspectItem()
{
    if(sender())
        emit inspectArchive(qobject_cast<ArchiveListWidgetItem *>(sender())->archive());
}

void ArchiveListWidget::restoreItem()
{
    ArchiveListWidgetItem *archiveItem = qobject_cast<ArchiveListWidgetItem *>(sender());
    if(archiveItem)
    {
        RestoreDialog restoreDialog(archiveItem->archive(), this);
        if(QDialog::Accepted == restoreDialog.exec())
            emit restoreArchive(archiveItem->archive(),
                                restoreDialog.getOptions());
    }
}

void ArchiveListWidget::goToJob()
{
    if(sender())
        emit displayJobDetails(
            qobject_cast<ArchiveListWidgetItem *>(sender())->archive()->jobRef());
}

void ArchiveListWidget::setSelectedArchive(ArchivePtr archive)
{
    if(!archive)
        return;

    ArchiveListWidgetItem *archiveItem = static_cast<ArchiveListWidgetItem *>(currentItem());
    if(!archiveItem || (archiveItem->archive() != archive))
    {
        for(int i = 0; i < count(); ++i)
        {
            ArchiveListWidgetItem *archiveItem =
                static_cast<ArchiveListWidgetItem *>(item(i));
            if(archiveItem &&
               (archiveItem->archive()->objectKey() == archive->objectKey()))
            {
                setCurrentItem(archiveItem);
            }
        }
    }
}

void ArchiveListWidget::disableArchives(QList<ArchivePtr> archives)
{
    for(int i = 0; i < count(); ++i)
    {
        ArchiveListWidgetItem *archiveItem = static_cast<ArchiveListWidgetItem *>(item(i));
        if(archiveItem)
        {
            foreach(ArchivePtr archive, archives)
            {
                if((archiveItem->archive()->objectKey() == archive->objectKey()))
                    archiveItem->setDisabled();
            }
        }
    }
}

void ArchiveListWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Delete:
        removeSelectedItems();
        break;
    case Qt::Key_Escape:
        if(!selectedItems().isEmpty())
            clearSelection();
        else
            QListWidget::keyPressEvent(event);
        break;
    default:
        QListWidget::keyPressEvent(event);
    }
}

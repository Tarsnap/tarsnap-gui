#include "archivelistwidget.h"
#include "archivelistwidgetitem.h"
#include "debug.h"
#include "restoredialog.h"

#include <QKeyEvent>
#include <QMessageBox>

#define DELETE_CONFIRMATION_THRESHOLD 10

ArchiveListWidget::ArchiveListWidget(QWidget *parent)
    : QListWidget(parent), _highlightedItem(nullptr)
{
    _filter.setCaseSensitivity(Qt::CaseInsensitive);
    _filter.setPatternSyntax(QRegExp::Wildcard);
    connect(this, &QListWidget::itemActivated, [&](QListWidgetItem *item) {
        if(item)
        {
            ArchiveListWidgetItem *archiveItem =
                static_cast<ArchiveListWidgetItem *>(item);
            if(archiveItem && !archiveItem->archive()->deleteScheduled())
                goingToInspectItem(archiveItem);
        }
    });
}

ArchiveListWidget::~ArchiveListWidget()
{
    clear();
}

void ArchiveListWidget::setArchives(QList<ArchivePtr> archives)
{
    std::sort(archives.begin(), archives.end(),
              [](const ArchivePtr &a, const ArchivePtr &b) {
                  return (a->timestamp() > b->timestamp());
              });
    setUpdatesEnabled(false);
    clear();
    for(const ArchivePtr &archive : archives)
        insertArchive(archive, count());
    setUpdatesEnabled(true);
}

void ArchiveListWidget::addArchive(ArchivePtr archive)
{
    if(!archive)
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    int pos = 0;
    for(; pos < count(); ++pos)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(pos));
        if(archiveItem
           && (archive->timestamp() > archiveItem->archive()->timestamp()))
        {
            break;
        }
    }
    insertArchive(archive, pos);
}

void ArchiveListWidget::deleteItem()
{
    ArchiveListWidgetItem *archiveItem =
        qobject_cast<ArchiveListWidgetItem *>(sender());
    if(archiveItem)
    {
        ArchivePtr archive = archiveItem->archive();

        QMessageBox::StandardButton confirm =
            QMessageBox::question(this, tr("Confirm delete"),
                                  tr("Are you sure you want to delete"
                                     " archive %1 (this cannot be undone)?")
                                      .arg(archive->name()));
        if(confirm == QMessageBox::Yes)
        {
            QList<ArchivePtr> archiveList;
            archiveList.append(archive);
            emit deleteArchives(archiveList);
        }
    }
}

void ArchiveListWidget::deleteSelectedItems()
{
    if(selectedItems().isEmpty())
        return;

    QList<ArchiveListWidgetItem *> selectedListItems;
    // Any archives pending deletion in the selection? if so deny action
    for(QListWidgetItem *item : selectedItems())
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item);
        if(!archiveItem || archiveItem->archive()->deleteScheduled())
            return;
        else
            selectedListItems << archiveItem;
    }

    int selectedItemsCount = selectedItems().count();

    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Confirm delete"),
                              tr("Are you sure you want to delete %1 "
                                 "selected archive(s) "
                                 "(this cannot be undone)?")
                                  .arg(selectedItemsCount));
    if(confirm != QMessageBox::Yes)
        return;

    // Some more deletion confirmation, if count of archives to be
    // removed is bigger than threshold
    if(selectedItemsCount >= DELETE_CONFIRMATION_THRESHOLD)
    {
        // Inform of purge operation if all archives are to be removed
        if(selectedItemsCount == count())
        {
            confirm =
                QMessageBox::question(this, tr("Confirm delete"),
                                      tr("Are you sure you want to delete "
                                         "all of your archives?\nFor your "
                                         "information, there's a purge "
                                         "action in Settings -> Account "
                                         "page that achieves the same "
                                         "thing but more efficiently."));
        }
        else
        {
            confirm = QMessageBox::question(this, tr("Confirm delete"),
                                            tr("This will permanently delete "
                                               "the %1 selected archives. "
                                               "Proceed?")
                                                .arg(selectedItemsCount));
        }
    }

    if(confirm == QMessageBox::Yes)
    {
        QList<ArchivePtr> archivesToDelete;
        for(ArchiveListWidgetItem *archiveItem : selectedListItems)
        {
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
        if(archiveItem && !archiveItem->archive()->deleteScheduled())
            goingToInspectItem(archiveItem);
    }
}

void ArchiveListWidget::restoreSelectedItem()
{
    if(!selectedItems().isEmpty())
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(selectedItems().first());
        if(archiveItem && !archiveItem->archive()->deleteScheduled())
        {
            RestoreDialog *restoreDialog =
                new RestoreDialog(this, archiveItem->archive());
            restoreDialog->show();
            connect(restoreDialog, &RestoreDialog::accepted, [=] {
                emit restoreArchive(restoreDialog->archive(),
                                    restoreDialog->getOptions());
            });
        }
    }
}

void ArchiveListWidget::setFilter(QString regex)
{
    setUpdatesEnabled(false);
    clearSelection();
    _filter.setPattern(regex);
    for(int i = 0; i < count(); ++i)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(i));
        if(archiveItem)
        {
            if(archiveItem->archive()->name().contains(_filter))
                archiveItem->setHidden(false);
            else
                archiveItem->setHidden(true);
        }
    }
    setUpdatesEnabled(true);
    emit countChanged(count(), visibleItemsCount());
}

void ArchiveListWidget::removeItem()
{
    ArchiveListWidgetItem *archiveItem =
        qobject_cast<ArchiveListWidgetItem *>(sender());
    if(archiveItem)
    {
        delete archiveItem; // Removes item from the list
        emit countChanged(count(), visibleItemsCount());
    }
}

void ArchiveListWidget::insertArchive(ArchivePtr archive, int pos)
{
    if(!archive)
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    ArchiveListWidgetItem *item = new ArchiveListWidgetItem(archive);
    connect(item, &ArchiveListWidgetItem::requestDelete, this,
            &ArchiveListWidget::deleteItem);
    connect(item, &ArchiveListWidgetItem::requestInspect, this,
            &ArchiveListWidget::inspectItem);
    connect(item, &ArchiveListWidgetItem::requestRestore, this,
            &ArchiveListWidget::restoreItem);
    connect(item, &ArchiveListWidgetItem::requestGoToJob, this,
            &ArchiveListWidget::goToJob);
    connect(item, &ArchiveListWidgetItem::removeItem, this,
            &ArchiveListWidget::removeItem);
    insertItem(pos, item);
    setItemWidget(item, item->widget());
    item->setHidden(!archive->name().contains(_filter));
    emit countChanged(count(), visibleItemsCount());
}

int ArchiveListWidget::visibleItemsCount()
{
    int count = 0;
    for(QListWidgetItem *item : findItems("*", Qt::MatchWildcard))
    {
        if(item && !item->isHidden())
            count++;
    }
    return count;
}

void ArchiveListWidget::inspectItem()
{
    if(sender())
        goingToInspectItem(qobject_cast<ArchiveListWidgetItem *>(sender()));
}

void ArchiveListWidget::restoreItem()
{
    ArchiveListWidgetItem *archiveItem =
        qobject_cast<ArchiveListWidgetItem *>(sender());
    if(archiveItem)
    {
        RestoreDialog *restoreDialog =
            new RestoreDialog(this, archiveItem->archive());
        restoreDialog->show();
        connect(restoreDialog, &RestoreDialog::accepted, [=] {
            emit restoreArchive(restoreDialog->archive(),
                                restoreDialog->getOptions());
        });
    }
}

void ArchiveListWidget::goToJob()
{
    if(sender())
        emit displayJobDetails(
            qobject_cast<ArchiveListWidgetItem *>(sender())->archive()->jobRef());
}

void ArchiveListWidget::selectArchive(ArchivePtr archive)
{
    if(!archive)
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    for(int i = 0; i < count(); ++i)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(i));
        if(archiveItem
           && (archiveItem->archive()->objectKey() == archive->objectKey()))
        {
            clearSelection();
            setCurrentItem(archiveItem);
            scrollToItem(currentItem(), QAbstractItemView::EnsureVisible);
            break;
        }
    }
}

void ArchiveListWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Delete:
        deleteSelectedItems();
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

void ArchiveListWidget::goingToInspectItem(ArchiveListWidgetItem *archiveItem)
{
    // Disable previous highlighting
    if(_highlightedItem != nullptr)
        _highlightedItem->setShowingDetails(false);

    // Toggle visibility (if already visible)
    if(_highlightedItem == archiveItem)
    {
        _highlightedItem = nullptr;
        emit clearInspectArchive();
        return;
    }

    // Highlight new item
    _highlightedItem = archiveItem;
    _highlightedItem->setShowingDetails(true);

    emit inspectArchive(archiveItem->archive());
}

void ArchiveListWidget::ensureCurrentItemVisible()
{
    scrollToItem(currentItem(), QAbstractItemView::EnsureVisible);
}

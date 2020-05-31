#include "archivelistwidget.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRegExp>
#include <Qt>
WARNINGS_ENABLE

#include "messages/archiverestoreoptions.h"

#include "debug.h"
#include "persistentmodel/archive.h"
#include "widgets/archivelistwidgetitem.h"
#include "widgets/restoredialog.h"

#define DELETE_CONFIRMATION_THRESHOLD 10

ArchiveListWidget::ArchiveListWidget(QWidget *parent)
    : QListWidget(parent), _filter(new QRegExp), _highlightedItem(nullptr)
{
    // Set up filtering archive names.
    _filter->setCaseSensitivity(Qt::CaseInsensitive);
    _filter->setPatternSyntax(QRegExp::Wildcard);

    // Connection for showing info about an Archive.
    connect(this, &QListWidget::itemActivated, this,
            &ArchiveListWidget::handleItemActivated);
}

ArchiveListWidget::~ArchiveListWidget()
{
    clear();
    delete _filter;
}

static bool cmp_timestamp(const ArchivePtr &a, const ArchivePtr &b)
{
    return (a->timestamp() > b->timestamp());
}

void ArchiveListWidget::setArchives(QList<ArchivePtr> archives)
{
    // Sort archive list.
    std::sort(archives.begin(), archives.end(), cmp_timestamp);

    // Clear existing list and add archives (in sorted order).
    setUpdatesEnabled(false);
    clear();
    for(const ArchivePtr &archive : archives)
        insertArchive(archive, count());
    setUpdatesEnabled(true);
}

void ArchiveListWidget::addArchive(ArchivePtr archive)
{
    // Bail (if applicable).
    if(!archive)
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    // Find insertion position based on sorted timestamps.
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
    // Get item requesting the deletion.
    ArchiveListWidgetItem *archiveItem =
        qobject_cast<ArchiveListWidgetItem *>(sender());

    // Bail (if applicable).
    if(!archiveItem)
        return;

    // Get archive name.
    ArchivePtr archive = archiveItem->archive();

    // Confirm deletion.
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Confirm delete"),
                              tr("Are you sure you want to delete"
                                 " archive %1 (this cannot be undone)?")
                                  .arg(archive->name()));
    if(confirm != QMessageBox::Yes)
        return;

    // Delete archive.
    QList<ArchivePtr> archiveList;
    archiveList.append(archive);
    emit deleteArchives(archiveList);
}

void ArchiveListWidget::deleteSelectedItems()
{
    // Bail (if applicable).
    if(selectedItems().isEmpty())
        return;

    // Any archives pending deletion in the selection? if so deny action
    for(QListWidgetItem *item : selectedItems())
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item);
        if(!archiveItem || archiveItem->archive()->deleteScheduled())
            return;
    }

    // Confirm deletion.
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
    if(confirm != QMessageBox::Yes)
        return;

    // Schedule archives for deletion.
    QList<ArchivePtr> archivesToDelete;
    for(QListWidgetItem *item : selectedItems())
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item);
        archivesToDelete.append(archiveItem->archive());
    }
    if(!archivesToDelete.isEmpty())
        emit deleteArchives(archivesToDelete);
}

void ArchiveListWidget::inspectSelectedItem()
{
    // Bail (if applicable).
    if(selectedItems().isEmpty())
        return;

    ArchiveListWidgetItem *archiveItem =
        static_cast<ArchiveListWidgetItem *>(selectedItems().first());
    if(archiveItem && !archiveItem->archive()->deleteScheduled())
        goingToInspectItem(archiveItem);
}

void ArchiveListWidget::restoreSelectedItem()
{
    // Bail (if applicable).
    if(selectedItems().isEmpty())
        return;

    // Get first selected item.
    ArchiveListWidgetItem *archiveItem =
        static_cast<ArchiveListWidgetItem *>(selectedItems().first());

    // Bail (if applicable).
    if(!archiveItem || archiveItem->archive()->deleteScheduled())
        return;

    // Launch RestoreDialog.
    RestoreDialog *restoreDialog =
        new RestoreDialog(this, archiveItem->archive());
    connect(restoreDialog, &RestoreDialog::accepted, [this, restoreDialog] {
        emit restoreArchive(restoreDialog->archive(),
                            restoreDialog->getOptions());
    });
    restoreDialog->show();
}

void ArchiveListWidget::setFilter(const QString &regex)
{
    // Set up filter.
    setUpdatesEnabled(false);
    clearSelection();
    _filter->setPattern(regex);

    // Check archives against filter.
    for(int i = 0; i < count(); ++i)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(i));
        if(archiveItem)
        {
            if(archiveItem->archive()->name().contains(*_filter))
                archiveItem->setHidden(false);
            else
                archiveItem->setHidden(true);
        }
    }
    setUpdatesEnabled(true);

    // Notify about the number of visible items.
    emit countChanged(count(), visibleItemsCount());
}

void ArchiveListWidget::removeItem()
{
    ArchiveListWidgetItem *archiveItem =
        qobject_cast<ArchiveListWidgetItem *>(sender());
    // Bail (if applicable).
    if(!archiveItem)
        return;

    // Remove item from the list.
    delete archiveItem;

    // Notify about the number of visible items.
    emit countChanged(count(), visibleItemsCount());
}

void ArchiveListWidget::insertArchive(ArchivePtr archive, int pos)
{
    // Bail (if applicable).
    if(!archive)
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    // Create new item.
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

    // Add it to the list at the indicated position.
    insertItem(pos, item);
    setItemWidget(item, item->widget());

    // Check it against the name filter.
    item->setHidden(!archive->name().contains(*_filter));

    // Notify about the number of visible items.
    emit countChanged(count(), visibleItemsCount());
}

int ArchiveListWidget::visibleItemsCount()
{
    // Find the number of items which are not hidden.
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
    // Bail (if applicable).
    if(!archiveItem)
        return;

    // Launch RestoreDialog.
    RestoreDialog *restoreDialog =
        new RestoreDialog(this, archiveItem->archive());
    connect(restoreDialog, &RestoreDialog::accepted, [this, restoreDialog] {
        emit restoreArchive(restoreDialog->archive(),
                            restoreDialog->getOptions());
    });
    restoreDialog->show();
}

void ArchiveListWidget::goToJob()
{
    // Notify that we want to display the Job.
    if(sender())
        emit displayJobDetails(qobject_cast<ArchiveListWidgetItem *>(sender())
                                   ->archive()
                                   ->jobRef());
}

void ArchiveListWidget::selectArchive(ArchivePtr archive)
{
    // Bail (if applicable).
    if(!archive)
    {
        DEBUG << "Null ArchivePtr passed.";
        return;
    }

    // Find the archive in the list.
    for(int i = 0; i < count(); ++i)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(i));
        if(archiveItem
           && (archiveItem->archive()->objectKey() == archive->objectKey()))
        {
            // Select the desired archive.
            clearSelection();
            setCurrentItem(archiveItem);

            // Make sure the scroll are includes the archive.
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

void ArchiveListWidget::noInspect()
{
    // Bail (if applicable).
    if(_highlightedItem == nullptr)
        return;

    // Indicate to the item that we're not showing it any more.
    _highlightedItem->setShowingDetails(false);
    _highlightedItem = nullptr;
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

void ArchiveListWidget::handleItemActivated(QListWidgetItem *item)
{
    ArchiveListWidgetItem *archiveItem =
        static_cast<ArchiveListWidgetItem *>(item);

    // Sanity check
    if(!archiveItem)
    {
        DEBUG << "ArchiveListWidget::handleItemActivated(nullptr)";
        return;
    }

    // Toggle showing details about the item if it's being deleted
    if(!archiveItem->archive()->deleteScheduled())
        goingToInspectItem(archiveItem);
}

ArchivePtr ArchiveListWidget::findArchiveByName(const QString &archiveName)
{
    // Find the archive name in the list.
    for(int i = 0; i < count(); i++)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(i));
        if(archiveName == archiveItem->archive()->name())
            return archiveItem->archive();
    }

    // We couldn't find the name.
#if(QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    return nullptr;
#else
    return ArchivePtr(nullptr);
#endif
}

void ArchiveListWidget::updateIEC()
{
    // Update all items.
    for(int i = 0; i < count(); i++)
    {
        ArchiveListWidgetItem *archiveItem =
            static_cast<ArchiveListWidgetItem *>(item(i));
        archiveItem->updateIEC();
    }
}

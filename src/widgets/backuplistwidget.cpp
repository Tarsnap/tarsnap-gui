#include "backuplistwidget.h"
#include "backuplistitem.h"
#include "debug.h"
#include "utils.h"

#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QSettings>

BackupListWidget::BackupListWidget(QWidget *parent):
    QListWidget(parent)
{
    QSettings settings;
    QStringList urls = settings.value("app/backup_list").toStringList();
    if(!urls.isEmpty())
    {
        QList<QUrl> urllist;
        foreach(QString url, urls)
            urllist << QUrl::fromUserInput(url);
        if(!urllist.isEmpty())
            QMetaObject::invokeMethod(this, "addItemsWithUrls"
                                      , QUEUED, Q_ARG(QList<QUrl>, urllist));
    }
    connect(this, &QListWidget::itemActivated,
    [&](QListWidgetItem * item)
    {
        static_cast<BackupListItem*>(item)->browseUrl();
    });
}

BackupListWidget::~BackupListWidget()
{
    QSettings settings;
    QStringList urls;
    for(int i = 0; i < this->count(); ++i)
    {
        BackupListItem *backupItem = static_cast<BackupListItem*>(item(i));
        urls << backupItem->url().toString(QUrl::FullyEncoded);
    }
    settings.setValue("app/backup_list", urls);
    settings.sync();
    clear();
}

void BackupListWidget::addItemWithUrl(QUrl url)
{
    if(!url.isEmpty())
    {
        QString fileUrl = url.toLocalFile();
        if(fileUrl.isEmpty())
            return;
        QFileInfo file(fileUrl);
        if(!file.exists())
            return;
        BackupListItem *item = new BackupListItem(url);
        connect(item, &BackupListItem::requestDelete, this, &BackupListWidget::removeItems);
        connect(item, &BackupListItem::requestUpdate, this, &BackupListWidget::recomputeListTotals);
        this->insertItem(this->count(), item);
        this->setItemWidget(item, item->widget());
    }
}

void BackupListWidget::addItemsWithUrls(QList<QUrl> urls)
{
    foreach(QUrl url, urls)
        addItemWithUrl(url);
    recomputeListTotals();
}

void BackupListWidget::removeItems()
{
    if(this->selectedItems().count() == 0)
    {
        // attempt to remove the sender
        BackupListItem* backupItem = qobject_cast<BackupListItem*>(sender());
        if(backupItem) delete backupItem;
    }
    else
    {
        foreach(QListWidgetItem *item, this->selectedItems())
        {
            if(item && item->isSelected())
                delete item;
        }
    }
    recomputeListTotals();
}

void BackupListWidget::recomputeListTotals()
{
    quint64 count = 0;
    quint64 size = 0;
    for(int i = 0; i < this->count(); ++i)
    {
        BackupListItem *backupItem = static_cast<BackupListItem*>(item(i));
        if(backupItem && (backupItem->count() != 0))
        {
            count += backupItem->count();
            size  += backupItem->size();
        }
    }
    emit itemTotals(count, size);
}

void BackupListWidget::dragMoveEvent( QDragMoveEvent* event )
{
    if ( !event->mimeData()->hasUrls() )
    {
        event->ignore();
        return;
    }
    event->accept();
}

void BackupListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BackupListWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty())
        addItemsWithUrls(urls);

    event->acceptProposedAction();
}

void BackupListWidget::keyReleaseEvent(QKeyEvent *event)
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


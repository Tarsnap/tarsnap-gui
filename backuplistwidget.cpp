#include "backuplistwidget.h"
#include "backuplistitem.h"

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QSettings>

/*FIXME: Remove after QTBUG-40449 is fixed in Qt5*/
#ifdef Q_OS_OSX
// for file drops from Finder, working around QTBUG-40449
namespace Platform {
extern QUrl osxRefToUrl(const QUrl &url);
} // Platform
#endif

BackupListWidget::BackupListWidget(QWidget *parent):
    QListWidget(parent)
{
    QSettings settings;
    QStringList urls = settings.value("app/backup_list").toStringList();
    if(!urls.isEmpty())
    {
        QList<QUrl> urllist;
        foreach (QString url, urls)
            urllist << QUrl::fromUserInput(url);
        if(!urllist.isEmpty())
            QMetaObject::invokeMethod(this, "addItemsWithUrls"
                                      , Qt::QueuedConnection, Q_ARG(QList<QUrl>, urllist));
    }
}

BackupListWidget::~BackupListWidget()
{
    QSettings settings;
    QStringList urls;
    for(int i = 0; i < this->count(); ++i)
    {
        BackupListItem *backupItem = dynamic_cast<BackupListItem*>(item(i));
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
        connect(item, SIGNAL(requestDelete()), this, SLOT(removeItems()));
        connect(item, SIGNAL(requestUpdate()), this, SLOT(recomputeListTotals()));
        this->insertItem(this->count(), item);
        this->setItemWidget(item, item->widget());
    }
}

void BackupListWidget::addItemsWithUrls(QList<QUrl> urls)
{
    foreach (QUrl url, urls) {
#ifdef Q_OS_OSX
/*FIXME: Remove after QTBUG-40449 is fixed in Qt5*/
        url = Platform::osxRefToUrl(url);
#endif
        addItemWithUrl(url);
    }
    recomputeListTotals();
}

void BackupListWidget::removeItems()
{
    if(this->selectedItems().count() == 0)
    {
        // attempt to remove the sender
        BackupListItem* backupItem = qobject_cast<BackupListItem*>(sender());
        if(backupItem)
        {
            QListWidgetItem *item = this->takeItem(this->row(backupItem));
            if(item)
                delete item;
        }
    }
    else
    {
        foreach (QListWidgetItem *item, this->selectedItems())
        {
            if(item->isSelected())
            {
                QListWidgetItem *takenItem = this->takeItem(this->row(item));
                if(takenItem)
                    delete takenItem;
            }
        }
    }
    recomputeListTotals();
}

void BackupListWidget::recomputeListTotals()
{
    qint64 count = 0;
    qint64 size = 0;
    for(int i = 0; i < this->count(); ++i)
    {
        BackupListItem *backupItem = dynamic_cast<BackupListItem*>(item(i));
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
        if(this->selectedItems().count() != 0)
            clearSelection();
        else
            QListWidget::keyReleaseEvent(event);
        break;
    default:
        QListWidget::keyReleaseEvent(event);
    }
}


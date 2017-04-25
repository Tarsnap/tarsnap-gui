#include "backuplistwidget.h"
#include "backuplistwidgetitem.h"
#include "debug.h"
#include "utils.h"

#include <QDropEvent>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>

BackupListWidget::BackupListWidget(QWidget *parent) : QListWidget(parent)
{
    QSettings   settings;
    QStringList urls = settings.value("app/backup_list").toStringList();
    if(!urls.isEmpty())
    {
        QList<QUrl> urllist;
        foreach(QString url, urls)
            urllist << QUrl::fromUserInput(url);
        if(!urllist.isEmpty())
            QMetaObject::invokeMethod(this, "addItemsWithUrls", QUEUED,
                                      Q_ARG(QList<QUrl>, urllist));
    }
    connect(this, &QListWidget::itemActivated, [&](QListWidgetItem *item) {
        static_cast<BackupListWidgetItem *>(item)->browseUrl();
    });
}

BackupListWidget::~BackupListWidget()
{
    QStringList urls;
    for(int i = 0; i < count(); ++i)
    {
        BackupListWidgetItem *backupItem = static_cast<BackupListWidgetItem *>(item(i));
        urls << backupItem->url().toString(QUrl::FullyEncoded);
    }
    QSettings settings;
    settings.setValue("app/backup_list", urls);
    settings.sync();
    clear();
}

void BackupListWidget::addItemWithUrl(QUrl url)
{
    if(url.isEmpty())
        return;

    QString fileUrl = url.toLocalFile();
    if(fileUrl.isEmpty())
        return;
    QFileInfo file(fileUrl);
    if(!file.exists())
        return;

    QList<QUrl> urls    = itemUrls();
    bool        matches = false;
    foreach(QUrl existingUrl, urls)
    {
        if(url == existingUrl)
        {
            matches = true;
            break;
        }
        QFileInfo existingFile(existingUrl.toLocalFile());
        if(existingFile.isDir() &&
           fileUrl.startsWith(existingFile.absoluteFilePath()))
        {
            matches = true;
            break;
        }
    }

    if(matches)
    {
        auto confirm =
            QMessageBox::question(this, tr("Confirm action"),
                                  tr("The file or directory:\n    %1\n"
                                     "was already in the backup list;"
                                     " adding it again will have no effect.\n"
                                     "Add anyway?").arg(url.toLocalFile()));
        if(confirm == QMessageBox::No)
            return;
    }

    BackupListWidgetItem *item = new BackupListWidgetItem(url);
    connect(item, &BackupListWidgetItem::requestDelete, this,
            &BackupListWidget::removeItems);
    connect(item, &BackupListWidgetItem::requestUpdate, this,
            &BackupListWidget::recomputeListTotals);
    insertItem(count(), item);
    setItemWidget(item, item->widget());
    emit itemWithUrlAdded(url);
}

void BackupListWidget::addItemsWithUrls(QList<QUrl> urls)
{
    setUpdatesEnabled(false);
    foreach(QUrl url, urls)
        addItemWithUrl(url);
    recomputeListTotals();
    setUpdatesEnabled(true);
}

void BackupListWidget::setItemsWithUrls(QList<QUrl> urls)
{
    setUpdatesEnabled(false);
    clear();
    setUpdatesEnabled(true);
    addItemsWithUrls(urls);
}

QList<QUrl> BackupListWidget::itemUrls()
{
    QList<QUrl> urls;
    for(int i = 0; i < count(); ++i)
    {
        BackupListWidgetItem *backupItem = static_cast<BackupListWidgetItem *>(item(i));
        urls << backupItem->url().toString(QUrl::FullyEncoded);
    }
    return urls;
}

void BackupListWidget::removeItems()
{
    setUpdatesEnabled(false);
    if(selectedItems().count() == 0)
    {
        // attempt to remove the sender
        BackupListWidgetItem *backupItem = qobject_cast<BackupListWidgetItem *>(sender());
        if(backupItem)
            delete backupItem;
    }
    else
    {
        foreach(QListWidgetItem *item, selectedItems())
        {
            if(item && item->isSelected())
                delete item;
        }
    }
    setUpdatesEnabled(true);
    recomputeListTotals();
}

void BackupListWidget::recomputeListTotals()
{
    quint64 items = 0;
    quint64 size  = 0;
    for(int i = 0; i < count(); ++i)
    {
        BackupListWidgetItem *backupItem = static_cast<BackupListWidgetItem *>(item(i));
        if(backupItem && (backupItem->count() != 0))
        {
            items += backupItem->count();
            size  += backupItem->size();
        }
    }
    emit itemTotals(items, size);
}

void BackupListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if(!event->mimeData()->hasUrls())
    {
        event->ignore();
        return;
    }
    event->accept();
}

void BackupListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BackupListWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if(!urls.isEmpty())
        addItemsWithUrls(urls);

    event->acceptProposedAction();
}

void BackupListWidget::keyPressEvent(QKeyEvent *event)
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
            QListWidget::keyPressEvent(event);
        break;
    default:
        QListWidget::keyPressEvent(event);
    }
}

void BackupListWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
        recomputeListTotals();
    QWidget::changeEvent(event);
}

#include "backuplistwidget.h"

WARNINGS_DISABLE
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QStringList>
#include <QUrl>
#include <QVariant>
#include <QWidget>
#include <Qt>
WARNINGS_ENABLE

#include "TSettings.h"

#include "basetask.h"
#include "widgets/backuplistwidgetitem.h"

BackupListWidget::BackupListWidget(QWidget *parent) : QListWidget(parent)
{
    loadBackupList();

    // Connection to open the file or dir with QDeskopServices::openUrl().
    connect(this, &QListWidget::itemActivated, [](QListWidgetItem *item) {
        static_cast<BackupListWidgetItem *>(item)->browseUrl();
    });
}

BackupListWidget::~BackupListWidget()
{
    saveBackupList();
    clear();
}

void BackupListWidget::saveBackupList()
{
    // Convert QUrls to strings.
    QStringList urls;
    for(int i = 0; i < count(); ++i)
    {
        BackupListWidgetItem *backupItem =
            static_cast<BackupListWidgetItem *>(item(i));
        urls << backupItem->url().toString(QUrl::FullyEncoded);
    }

    // Save list.
    TSettings settings;
    settings.setValue("app/backup_list", urls);
    settings.sync();
}

void BackupListWidget::loadBackupList()
{
    // Load previous backup list from Settings.
    TSettings   settings;
    QStringList urls =
        settings.value("app/backup_list", QStringList()).toStringList();

    // Bail if nothing to do.
    if(urls.isEmpty())
        return;

    // Convert url strings to QUrls.
    QList<QUrl> urllist;
    for(const QString &url : urls)
        urllist << QUrl::fromUserInput(url);

    // Load the list in the background, via the event loop.
    if(!urllist.isEmpty())
        QMetaObject::invokeMethod(this, "addItemsWithUrls",
                                  Qt::QueuedConnection,
                                  Q_ARG(QList<QUrl>, urllist));
}

void BackupListWidget::addItemWithUrl(const QUrl &url)
{
    // Bail if nothing to do.
    if(url.isEmpty())
        return;

    // Bail if the filename is empty, or the file doesn't exist.
    QString fileUrl = url.toLocalFile();
    if(fileUrl.isEmpty())
        return;
    QFileInfo file(fileUrl);
    if(!file.exists())
        return;

    // Find out if the new url matches any of the current urls.
    QList<QUrl> urls    = itemUrls();
    bool        matches = false;
    for(const QUrl &existingUrl : urls)
    {
        if(url == existingUrl)
        {
            matches = true;
            break;
        }
        QFileInfo existingFile(existingUrl.toLocalFile());
        if(existingFile.isDir()
           && fileUrl.startsWith(existingFile.absoluteFilePath()))
        {
            matches = true;
            break;
        }
    }

    // If any matches exist, ask user for confirmation.
    if(matches)
    {
        QMessageBox::StandardButton confirm =
            QMessageBox::question(this, tr("Confirm action"),
                                  tr("The file or directory:\n    %1\n"
                                     "was already in the backup list;"
                                     " adding it again will have no effect.\n"
                                     "Add anyway?")
                                      .arg(url.toLocalFile()));
        if(confirm == QMessageBox::No)
            return;
    }

    // Create a new item for the url.
    BackupListWidgetItem *item = new BackupListWidgetItem();
    connect(item, &BackupListWidgetItem::requestDelete, this,
            &BackupListWidget::removeItems);
    connect(item, &BackupListWidgetItem::requestUpdate, this,
            &BackupListWidget::recomputeListTotals);
    connect(item, &BackupListWidgetItem::taskRequested, this,
            &BackupListWidget::taskRequested);
    connect(item, &BackupListWidgetItem::cancelTaskRequested, this,
            &BackupListWidget::cancelTaskRequested);
    item->setUrl(url);

    // Append it to the list.
    insertItem(count(), item);
    setItemWidget(item, item->widget());

    // Notify that we've finished adding the url.
    emit itemWithUrlAdded(url);
}

void BackupListWidget::addItemsWithUrls(const QList<QUrl> &urls)
{
    setUpdatesEnabled(false);
    for(const QUrl &url : urls)
        addItemWithUrl(url);
    recomputeListTotals();
    setUpdatesEnabled(true);
}

void BackupListWidget::setItemsWithUrls(const QList<QUrl> &urls)
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
        BackupListWidgetItem *backupItem =
            static_cast<BackupListWidgetItem *>(item(i));
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
        BackupListWidgetItem *backupItem =
            qobject_cast<BackupListWidgetItem *>(sender());
        if(backupItem)
            delete backupItem;
    }
    else
    {
        // I'm not 100% certain that the below loop doesn't modify this value,
        // so I'm making a copy to be safe.
        const QList<QListWidgetItem *> items = selectedItems();
        for(QListWidgetItem *item : items)
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
        BackupListWidgetItem *backupItem =
            static_cast<BackupListWidgetItem *>(item(i));
        if(backupItem && (backupItem->count() != 0))
        {
            items += backupItem->count();
            size += backupItem->size();
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

void BackupListWidget::updateIEC()
{
    for(int i = 0; i < count(); ++i)
    {
        BackupListWidgetItem *backupItem =
            static_cast<BackupListWidgetItem *>(item(i));
        backupItem->updateIEC();
    }
}

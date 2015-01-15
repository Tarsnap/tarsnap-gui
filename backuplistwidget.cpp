#include "backuplistwidget.h"
#include "backuplistitem.h"

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>

/*FIXME: Remove after QTBUG-40449 is fixed in Qt5*/
#ifdef Q_OS_OSX
// for file drops from Finder, working around QTBUG-40449
namespace Utils {
namespace Platform {
extern QUrl osxRefToUrl(const QUrl &url);
} // Platform
} // Utils
#endif

BackupListWidget::BackupListWidget(QWidget *parent):
    QListWidget(parent)
{
}

BackupListWidget::~BackupListWidget()
{
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
        connect(item, SIGNAL(requestDelete()), this, SLOT(removeItem()));
        this->insertItem(this->count(), item);
        this->setItemWidget(item, item->widget());
    }
}

void BackupListWidget::removeItem()
{
//    QWidget *widget = this->itemWidget(qobject_cast<BackupListItem*>(sender()));
    QListWidgetItem *item = this->takeItem(this->row(qobject_cast<BackupListItem*>(sender())));
    Q_UNUSED(item)
    BackupListItem *backupItem = qobject_cast<BackupListItem*>(sender());
    backupItem->cleanup();
    delete backupItem;
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
    if (urls.isEmpty())
        return;

    foreach (QUrl url, urls) {
#ifdef Q_OS_OSX
/*FIXME: Remove after QTBUG-40449 is fixed in Qt5*/
    url = Utils::Platform::osxRefToUrl(url);
#endif
        addItemWithUrl(url);
    }

    event->acceptProposedAction();
}


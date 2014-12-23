#include "backuplistwidget.h"
#include "ui_backupitemwidget.h"
#include <QDropEvent>
#include <QMimeData>

BackupListWidget::BackupListWidget(QWidget *parent):
    _parent(parent)
{

}

BackupListWidget::~BackupListWidget()
{

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
        QString fileName = url.toLocalFile();
        if (fileName.isEmpty())
            return;
        Ui::BackupItemWidget backupItemUi;
        QListWidgetItem *item = new QListWidgetItem;
        QWidget *widget = new QWidget;
        backupItemUi.setupUi(widget);
        backupItemUi.pathLabel->setText(fileName);
        this->insertItem(0, item);
        this->setItemWidget(item, widget);
    }

    event->acceptProposedAction();
}



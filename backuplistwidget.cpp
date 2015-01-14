#include "backuplistwidget.h"
#include "ui_backupitemwidget.h"

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
    _parent(parent)
{

}

BackupListWidget::~BackupListWidget()
{

}

void BackupListWidget::addUrl(QUrl url)
{
    if(!url.isEmpty())
    {
        QString fileUrl = url.toLocalFile();
        if(fileUrl.isEmpty())
            return;
        QFileInfo file(fileUrl);
        if(!file.exists())
            return;
        Ui::BackupItemWidget itemUi;
        QListWidgetItem *item = new QListWidgetItem;
        QWidget *widget = new QWidget;
        itemUi.setupUi(widget);
        itemUi.pathLabel->setText(fileUrl);
        if(file.isDir())
        {
            QPixmap icon(":/resources/folder-2x.png");
            itemUi.iconLabel->setPixmap(icon);
            QString detail;
            QDir dir(file.absoluteFilePath());
            detail = QString::number(getDirCount(dir)) + " items totaling "
                   + QString::number(getDirSize(dir)) + " bytes";
            itemUi.detailsLabel->setText(detail);
        }
        else if(file.isFile())
        {
            QPixmap icon(":/resources/file-2x.png");
            itemUi.iconLabel->setPixmap(icon);
            itemUi.detailsLabel->setText(QString::number(file.size()) + " bytes");
        }
        else
        {
            // could be a device file, fifo or whatever, thus ignore
            return;
        }
        this->insertItem(this->count(), item);
        this->setItemWidget(item, widget);
    }
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
        addUrl(url);
    }

    event->acceptProposedAction();
}

qint64 BackupListWidget::getDirSize(QDir dir)
{
    qint64 result = 0;
    if(dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                qDebug() << "Traversing " << fileInfo.absoluteFilePath();
                result += getDirSize(QDir(fileInfo.absoluteFilePath()));
            }
            else
                result += fileInfo.size();
        }
    }
    return result;
}

qint64 BackupListWidget::getDirCount(QDir dir)
{
    qint64 count = 0;
    if(dir.exists())
    {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            if(fileInfo.isDir())
            {
                qDebug() << "Traversing " << fileInfo.absoluteFilePath();
                count += getDirCount(QDir(fileInfo.absoluteFilePath()));
            }
            ++count;
        }
    }
    return count;
}



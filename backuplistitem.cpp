#include "backuplistitem.h"

#include <QDebug>
#include <QFileInfo>
#include <QDesktopServices>

BackupListItem::BackupListItem(QUrl url): _widget(new QWidget())
{
    _ui.setupUi(_widget);
    connect(_ui.removeButton, SIGNAL(clicked()), this, SIGNAL(requestDelete()));
    connect(_ui.browseButton, SIGNAL(clicked()), this, SLOT(browseUrl()));
    setUrl(url);
}

BackupListItem::~BackupListItem()
{
    // we specifically don't delete _widget here since QListWidget::clear()
    // does that on its own, thus resulting in a double free
    // use cleanup() instead if deleted manually
}

QWidget* BackupListItem::widget()
{
    return _widget;
}
QUrl BackupListItem::url() const
{
    return _url;
}

void BackupListItem::setUrl(const QUrl &url)
{
    _url = url;

    if(!_url.isEmpty())
    {
        QString fileUrl = _url.toLocalFile();
        if(fileUrl.isEmpty())
            return;
        QFileInfo file(fileUrl);
        if(!file.exists())
            return;
        _ui.pathLabel->setText(fileUrl);
        if(file.isDir())
        {
            QPixmap icon(":/resources/folder-2x.png");
            _ui.iconLabel->setPixmap(icon);
            QString detail;
            QDir dir(file.absoluteFilePath());
            detail = QString::number(getDirCount(dir)) + " items totaling "
                   + QString::number(getDirSize(dir)) + " bytes";
            _ui.detailLabel->setText(detail);
        }
        else if(file.isFile())
        {
            QPixmap icon(":/resources/file-2x.png");
            _ui.iconLabel->setPixmap(icon);
            _ui.detailLabel->setText(QString::number(file.size()) + " bytes");
        }
        else
        {
            // could be a device file, fifo or whatever, thus ignore
            return;
        }
    }
}

void BackupListItem::cleanup()
{
    qDebug() << "CLEANUP";
    delete _widget;
}

void BackupListItem::browseUrl()
{
    QDesktopServices::openUrl(_url);
}

qint64 BackupListItem::getDirSize(QDir dir)
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

qint64 BackupListItem::getDirCount(QDir dir)
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




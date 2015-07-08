#include "backuplistitem.h"
#include "utils.h"

#include <QDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QThreadPool>
#include <QSettings>

BackupListItem::BackupListItem(QUrl url):_count(0), _size(0), _useSIPrefixes(false)
{
    _ui.setupUi(&_widget);
    _widget.addAction(_ui.actionOpen);
    _widget.addAction(_ui.actionRemove);
    _ui.browseButton->setDefaultAction(_ui.actionOpen);
    _ui.removeButton->setDefaultAction(_ui.actionRemove);
    connect(_ui.actionRemove, SIGNAL(triggered()), this, SIGNAL(requestDelete()), Qt::QueuedConnection);
    connect(_ui.actionOpen, SIGNAL(triggered()), this, SLOT(browseUrl()), Qt::QueuedConnection);

    QSettings settings;
    _useSIPrefixes = settings.value("app/si_prefixes", false).toBool();

    setUrl(url);
}

BackupListItem::~BackupListItem()
{
}

QWidget* BackupListItem::widget()
{
    return &_widget;
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
        _ui.pathLabel->setToolTip(fileUrl);
        if(file.isDir())
        {
            QPixmap icon(":/resources/icons/folder-2x.png");
            _ui.iconLabel->setPixmap(icon);
            QDir dir(file.absoluteFilePath());
            QThreadPool *threadPool = QThreadPool::globalInstance();
            Utils::GetDirInfoTask *task = new Utils::GetDirInfoTask(dir);
            task->setAutoDelete(true);
            connect(task, SIGNAL(result(quint64, quint64)), this, SLOT(updateDirDetail(quint64, quint64)), Qt::QueuedConnection);
            threadPool->start(task);
        }
        else if(file.isFile())
        {
            QPixmap icon(":/resources/icons/file-2x.png");
            _ui.iconLabel->setPixmap(icon);
            _count = 1;
            _size  = file.size();
            _ui.detailLabel->setText(Utils::humanBytes(_size, _useSIPrefixes));
        }
        else
        {
            // could be a device file, fifo or whatever, thus ignore
            return;
        }
    }
}

void BackupListItem::browseUrl()
{
    QDesktopServices::openUrl(_url);
}

void BackupListItem::updateDirDetail(quint64 size, quint64 count)
{
    _size = size;
    _count = count;
    _ui.detailLabel->setText(QString::number(_count) + tr(" items totalling ")
                             + Utils::humanBytes(_size, _useSIPrefixes));
    emit requestUpdate();
}
quint64 BackupListItem::size() const
{
    return _size;
}

void BackupListItem::setSize(const quint64 &size)
{
    _size = size;
}

quint64 BackupListItem::count() const
{
    return _count;
}

void BackupListItem::setCount(const quint64 &count)
{
    _count = count;
}


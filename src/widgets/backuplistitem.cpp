#include "backuplistitem.h"
#include "utils.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QSettings>
#include <QThreadPool>

BackupListItem::BackupListItem(QUrl url)
    : _widget(new QWidget), _count(0), _size(0), _useSIPrefixes(false)
{
    _ui.setupUi(_widget);
    _widget->addAction(_ui.actionOpen);
    _widget->addAction(_ui.actionRemove);
    _ui.browseButton->setDefaultAction(_ui.actionOpen);
    _ui.removeButton->setDefaultAction(_ui.actionRemove);
    connect(_ui.actionRemove, &QAction::triggered, this,
            &BackupListItem::requestDelete);
    connect(_ui.actionOpen, &QAction::triggered, this,
            &BackupListItem::browseUrl);

    QSettings settings;
    _useSIPrefixes = settings.value("app/si_prefixes", false).toBool();

    setUrl(url);
}

BackupListItem::~BackupListItem()
{
}

QWidget *BackupListItem::widget()
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
        _ui.pathLabel->setToolTip(fileUrl);
        if(file.isDir())
        {
            QPixmap icon(":/icons/folder.png");
            _ui.iconLabel->setPixmap(icon);
            QDir dir(file.absoluteFilePath());
            QThreadPool *threadPool = QThreadPool::globalInstance();
            Utils::GetDirInfoTask *task = new Utils::GetDirInfoTask(dir);
            task->setAutoDelete(true);
            connect(task, &Utils::GetDirInfoTask::result, this,
                    &BackupListItem::updateDirDetail, QUEUED);
            threadPool->start(task);
        }
        else if(file.isFile())
        {
            QPixmap icon(":/icons/file.png");
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
    _size  = size;
    _count = count;
    _ui.detailLabel->setText(QString::number(_count) + tr(" items totalling ") +
                             Utils::humanBytes(_size, _useSIPrefixes));
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

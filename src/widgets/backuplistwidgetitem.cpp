#include "backuplistwidgetitem.h"
#include "utils.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QThreadPool>

BackupListWidgetItem::BackupListWidgetItem(QUrl url)
    : _widget(new QWidget), _count(0), _size(0)
{
    _widget->installEventFilter(this);
    _ui.setupUi(_widget);
    _widget->addAction(_ui.actionOpen);
    _widget->addAction(_ui.actionRemove);
    _ui.browseButton->setDefaultAction(_ui.actionOpen);
    _ui.removeButton->setDefaultAction(_ui.actionRemove);
    _ui.removeButton->setToolTip(_ui.removeButton->toolTip()
                                   .arg(_ui.actionRemove->shortcut()
                                        .toString(QKeySequence::NativeText)));
    connect(_ui.actionRemove, &QAction::triggered, this,
            &BackupListWidgetItem::requestDelete);
    connect(_ui.actionOpen, &QAction::triggered, this,
            &BackupListWidgetItem::browseUrl);

    setUrl(url);
}

BackupListWidgetItem::~BackupListWidgetItem()
{
}

QWidget *BackupListWidgetItem::widget()
{
    return _widget;
}

QUrl BackupListWidgetItem::url() const
{
    return _url;
}

void BackupListWidgetItem::setUrl(const QUrl &url)
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
                    &BackupListWidgetItem::updateDirDetail, QUEUED);
            threadPool->start(task);
        }
        else if(file.isFile())
        {
            QPixmap icon(":/icons/file.png");
            _ui.iconLabel->setPixmap(icon);
            _count = 1;
            _size  = file.size();
            _ui.detailLabel->setText(Utils::humanBytes(_size));
        }
        else
        {
            // could be a device file, fifo or whatever, thus ignore
            return;
        }
    }
}

void BackupListWidgetItem::browseUrl()
{
    QDesktopServices::openUrl(_url);
}

void BackupListWidgetItem::updateDirDetail(quint64 size, quint64 count)
{
    _size  = size;
    _count = count;
    _ui.detailLabel->setText(QString::number(_count) + tr(" items totalling ")
                             + Utils::humanBytes(_size));
    emit requestUpdate();
}

bool BackupListWidgetItem::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _widget) && (event->type() == QEvent::LanguageChange))
    {
        _ui.retranslateUi(_widget);
        return true;
    }
    return false;
}

quint64 BackupListWidgetItem::size() const
{
    return _size;
}

quint64 BackupListWidgetItem::count() const
{
    return _count;
}

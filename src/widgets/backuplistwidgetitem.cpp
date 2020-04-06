#include "backuplistwidgetitem.h"

WARNINGS_DISABLE
#include <QDesktopServices>
#include <QFileInfo>
#include <QThreadPool>

#include "ui_backuplistwidgetitem.h"
WARNINGS_ENABLE

#include "dirinfotask.h"
#include "utils.h"

BackupListWidgetItem::BackupListWidgetItem(QUrl url)
    : _ui(new Ui::BackupListWidgetItem),
      _widget(new QWidget),
      _count(0),
      _size(0)
{
    _ui->setupUi(_widget);
    // Send translation events to the widget.
    _widget->installEventFilter(this);
    updateUi();
    // Set a sensible size.
    setSizeHint(QSize(_widget->minimumWidth(), _widget->minimumHeight()));

    // Set up actions.
    _widget->addAction(_ui->actionOpen);
    _widget->addAction(_ui->actionRemove);
    _ui->browseButton->setDefaultAction(_ui->actionOpen);
    _ui->removeButton->setDefaultAction(_ui->actionRemove);
    // Set up action connections.
    connect(_ui->actionRemove, &QAction::triggered, this,
            &BackupListWidgetItem::requestDelete);
    connect(_ui->actionOpen, &QAction::triggered, this,
            &BackupListWidgetItem::browseUrl);
    // Begin loading the url.
    setUrl(url);
}

BackupListWidgetItem::~BackupListWidgetItem()
{
    delete _ui;
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
        // Display path in the widget.  pathLabel is an TElidedLabel, so
        // the text might be cut short.  Therefore we display the path
        // as a tooltip as well, which will always be displayed in full.
        _ui->pathLabel->setText(fileUrl);
        _ui->pathLabel->setToolTip(fileUrl);

        if(file.isDir())
        {
            QPixmap icon(":/icons/folder.png");
            _ui->iconLabel->setPixmap(icon);
            // Load info about this directory in a separate thread.
            QDir                   dir(file.absoluteFilePath());
            QThreadPool *          threadPool = QThreadPool::globalInstance();
            Utils::GetDirInfoTask *task       = new Utils::GetDirInfoTask(dir);
            task->setAutoDelete(true);
            connect(task, &Utils::GetDirInfoTask::result, this,
                    &BackupListWidgetItem::updateDirDetail, QUEUED);
            threadPool->start(task);
        }
        else if(file.isFile())
        {
            QPixmap icon(":/icons/file.png");
            _ui->iconLabel->setPixmap(icon);
            _count = 1;
            _size  = static_cast<quint64>(file.size());
            _ui->detailLabel->setText(Utils::humanBytes(_size));
        }
        else
        {
            // Could be a device file, fifo, or whatever; thus ignore.
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
    _ui->detailLabel->setText(QString::number(_count) + tr(" items, ")
                              + Utils::humanBytes(_size));
    emit requestUpdate();
}

bool BackupListWidgetItem::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _widget) && (event->type() == QEvent::LanguageChange))
    {
        _ui->retranslateUi(_widget);
        updateUi();
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

void BackupListWidgetItem::updateUi()
{
    // Display tooltip using a platform-specific string.
    _ui->actionRemove->setToolTip(_ui->actionRemove->toolTip().arg(
        _ui->actionRemove->shortcut().toString(QKeySequence::NativeText)));
}

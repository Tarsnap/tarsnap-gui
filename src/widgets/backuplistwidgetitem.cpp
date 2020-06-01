#include "backuplistwidgetitem.h"

WARNINGS_DISABLE
#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QMovie>
#include <QPixmap>
#include <QSize>
#include <QWidget>

#include "ui_backuplistwidgetitem.h"
WARNINGS_ENABLE

#include "TElidedLabel.h"

#include "basetask.h"
#include "dirinfotask.h"
#include "humanbytes.h"
#include "widgets/elidedclickablelabel.h"

#define BUSY_MOVIE_FILENAME ":/lib/loading.gif"

BackupListWidgetItem::BackupListWidgetItem()
    : _ui(new Ui::BackupListWidgetItem),
      _widget(new QWidget),
      _count(0),
      _size(0),
      _dirInfoTask(nullptr)
{
    _ui->setupUi(_widget);
    // Send translation events to the widget.
    _widget->installEventFilter(this);
    updateKeyboardShortcutInfo();
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

    // Set up the "busy" animation.
    _busyMovie = new QMovie(BUSY_MOVIE_FILENAME);
}

BackupListWidgetItem::~BackupListWidgetItem()
{
    // We don't need the task fo finish if we're deleting this item.
    if(_dirInfoTask != nullptr)
        emit cancelTaskRequested(_dirInfoTask, _dirInfoTaskUuid);

    _busyMovie->stop();
    _ui->detailLabel->setMovie(nullptr);
    delete _busyMovie;
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

    // Bail (if applicable).
    if(url.isEmpty())
        return;

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
        startDirInfoTask();
    }
    else if(file.isFile())
    {
        QPixmap icon(":/icons/file.png");
        _ui->iconLabel->setPixmap(icon);
        _count = 1;
        _size  = static_cast<quint64>(file.size());

        _size_prefix = "";
        updateIEC();
    }
    else
    {
        // Could be a device file, fifo, or whatever; thus ignore.
        return;
    }
}

void BackupListWidgetItem::updateIEC()
{
    _ui->detailLabel->setText(_size_prefix + humanBytes(_size));
}

void BackupListWidgetItem::browseUrl()
{
    QDesktopServices::openUrl(_url);
}

void BackupListWidgetItem::updateDirDetail(quint64 size, quint64 count)
{
    // The task has finished.
    _dirInfoTask = nullptr;
    _busyMovie->stop();
    _ui->detailLabel->setMovie(nullptr);

    _size  = size;
    _count = count;

    _size_prefix = QString::number(_count) + tr(" items, ");
    updateIEC();
    emit requestUpdate();
}

void BackupListWidgetItem::cancelDirDetail()
{
    // The task has finished.
    _dirInfoTask = nullptr;

    _ui->detailLabel->setText("canceled");
    _ui->detailLabel->setToolTip(
        "Calculation canceled by user; click to restart");
    connect(_ui->detailLabel, &ElidedClickableLabel::clicked, this,
            &BackupListWidgetItem::startDirInfoTask);
}

void BackupListWidgetItem::startDirInfoTask()
{
    // Cancel the clickable label.
    disconnect(_ui->detailLabel, &ElidedClickableLabel::clicked, this,
               &BackupListWidgetItem::startDirInfoTask);

    // Bail if we're already working on this.
    if(_dirInfoTask != nullptr)
        return;

    // Get the directory name, or bail.
    QString fileUrl = _url.toLocalFile();
    if(fileUrl.isEmpty())
        return;
    QFileInfo file(fileUrl);
    if(!file.exists())
        return;
    if(!file.isDir())
        return;

    // Prepare the task.
    QDir dir(file.absoluteFilePath());
    _dirInfoTask     = new DirInfoTask(dir);
    _dirInfoTaskUuid = _dirInfoTask->uuid();
    connect(_dirInfoTask, &DirInfoTask::result, this,
            &BackupListWidgetItem::updateDirDetail, Qt::QueuedConnection);
    connect(_dirInfoTask, &DirInfoTask::canceled, this,
            &BackupListWidgetItem::cancelDirDetail, Qt::QueuedConnection);

    // Display "waiting"
    _ui->detailLabel->setText("");
    _ui->detailLabel->setToolTip("");
    _ui->detailLabel->setMovie(_busyMovie);
    _busyMovie->start();

    // Send the task to the TaskManager.
    emit taskRequested(_dirInfoTask);
}

bool BackupListWidgetItem::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _widget) && (event->type() == QEvent::LanguageChange))
    {
        _ui->retranslateUi(_widget);
        updateKeyboardShortcutInfo();
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

void BackupListWidgetItem::updateKeyboardShortcutInfo()
{
    // Display tooltip using a platform-specific string.
    _ui->actionRemove->setToolTip(_ui->actionRemove->toolTip().arg(
        _ui->actionRemove->shortcut().toString(QKeySequence::NativeText)));
}

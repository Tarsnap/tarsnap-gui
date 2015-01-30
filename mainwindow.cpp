#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_archiveitemwidget.h"
#include "ui_backupitemwidget.h"
#include "backuplistitem.h"

#include <QPainter>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QSharedPointer>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::MainWindow),
    _loadingAnimation(":/resources/loading.gif")
{
    _ui->setupUi(this);

    _ui->backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->browseListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    _tarsnapLogo = new QLabel(this);
    QPixmap logo(":/resources/tarsnap.png");
    _tarsnapLogo->setPixmap(logo.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _tarsnapLogo->adjustSize();
    _tarsnapLogo->lower();
    _tarsnapLogo->show();

    _ui->loadingIconLabel->setMovie(&_loadingAnimation);

    _ui->archiveDetailsWidget->hide();

    readSettings();

    Ui::ArchiveItemWidget restoreItemUi;
    for(int i = 0; i < 10; i++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        QWidget *widget = new QWidget;
        restoreItemUi.setupUi(widget);
        _ui->backupRestoreListWidget->insertItem(i, item);
        _ui->backupRestoreListWidget->setItemWidget(item, widget);
    }

    _ui->browseListWidget->addAction(_ui->actionRefresh);
    connect(_ui->actionRefresh, SIGNAL(triggered()), _ui->browseListWidget
            ,SIGNAL(getArchivesList()), Qt::QueuedConnection);

    connect(_ui->backupListWidget, SIGNAL(itemTotals(qint64,qint64)), this
            , SLOT(updateBackupItemTotals(qint64, qint64)));
    connect(_ui->browseListWidget, SIGNAL(getArchivesList()), this, SIGNAL(getArchivesList()));
    connect(this, SIGNAL(archivesList(QList<ArchivePtr >))
            , _ui->browseListWidget, SLOT(addArchives(QList<ArchivePtr >)));
    connect(_ui->browseListWidget, SIGNAL(inspectArchive(ArchivePtr)), this
            , SLOT(displayInspectArchive(ArchivePtr)));
    connect(_ui->browseListWidget, SIGNAL(deleteArchive(ArchivePtr)), this
            , SIGNAL(deleteArchive(ArchivePtr)));
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::readSettings()
{
    QSettings settings;
    _ui->tarsnapUserLineEdit->setText(settings.value("tarsnap/user", "").toString());
    _ui->tarsnapKeyLineEdit->setText(settings.value("tarsnap/key", "").toString());
    _ui->tarsnapMachineLineEdit->setText(settings.value("tarsnap/machine", "").toString());
    _ui->tarsnapPathLineEdit->setText(settings.value("tarsnap/path", "").toString());
    _ui->tarsnapCacheLineEdit->setText(settings.value("tarsnap/cache", "").toString());
}


void MainWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    _tarsnapLogo->move(this->width()-_tarsnapLogo->width()-10,3);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _windowDragPos = event->pos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - _windowDragPos;
        QPoint newpos = this->pos() + diff;
        this->move(newpos);
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(isMaximized())
        this->showNormal();
    else
        this->showMaximized();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if((_ui->mainTabWidget->currentWidget() == _ui->browseTab)
           &&(_ui->archiveDetailsWidget->isVisible()))
            _ui->archiveDetailsWidget->hide();
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
}

void MainWindow::backupJobUpdate(BackupJobPtr job)
{
    switch (job->status) {
    case JobStatus::Completed:
        _ui->statusBarLabel->setText(tr("Job <i>%1</i> completed.").arg(job->name));
        _ui->statusBarLabel->setToolTip(_ui->statusBarLabel->text());
        break;
    case JobStatus::Started:
        _ui->statusBarLabel->setText(tr("Job <i>%1</i> created.").arg(job->name));
        _ui->statusBarLabel->setToolTip(_ui->statusBarLabel->text());
        break;
    case JobStatus::Running:
        _ui->statusBarLabel->setText(tr("Job <i>%1</i> is running.").arg(job->name));
        _ui->statusBarLabel->setToolTip(_ui->statusBarLabel->text());
        break;
    case JobStatus::Failed:
        _ui->statusBarLabel->setText(tr("Job <i>%1</i> failed: %2").arg(job->name).arg(job->output.simplified()));
        _ui->statusBarLabel->setToolTip(tr("%1\n%2").arg(job->reason).arg(job->output));
        break;
    case JobStatus::Paused:
        _ui->statusBarLabel->setText(tr("Job <i>%1</i> paused.").arg(job->name));
        _ui->statusBarLabel->setToolTip(_ui->statusBarLabel->text());
        break;
    default:
        break;
    }
}

void MainWindow::archiveDeleted(ArchivePtr archive)
{
    _ui->statusBarLabel->setText(tr("Archive <i>%1</i> deleted.").arg(archive->name));
}

void MainWindow::updateLoadingAnimation(bool idle)
{
    if(idle)
    {
        _loadingAnimation.stop();
        _ui->loadingIconLabel->hide();
    }
    else
    {
        _loadingAnimation.start();
        _ui->loadingIconLabel->show();
    }
}

void MainWindow::updateBackupItemTotals(qint64 count, qint64 size)
{
    if(count != 0)
    {
        _ui->backupDetailLabel->setText(tr("%1 items (%2 bytes)").arg(count).arg(size));
        _ui->backupButton->setEnabled(true);
    }
    else
    {
        _ui->backupDetailLabel->clear();
        _ui->backupButton->setEnabled(false);
    }
}

void MainWindow::displayInspectArchive(ArchivePtr archive)
{
    if(_currentArchiveDetail)
        disconnect(_currentArchiveDetail.data(), SIGNAL(changed()), this, SLOT(updateInspectArchive()));

    _currentArchiveDetail = archive;

    if(_currentArchiveDetail)
        connect(_currentArchiveDetail.data(), SIGNAL(changed()), this, SLOT(updateInspectArchive()));

    if(archive->sizeTotal == 0)
        emit loadArchiveStats(archive);

    if(archive->contents.count() == 0)
        emit loadArchiveContents(archive);

    updateInspectArchive();

    if(!_ui->archiveDetailsWidget->isVisible())
        _ui->archiveDetailsWidget->show();
}

void MainWindow::on_appendTimestampCheckBox_toggled(bool checked)
{
    if(checked)
    {
        QString text = _ui->backupNameLineEdit->text();
        _lastTimestamp.clear();
        _lastTimestamp.append("_");
        _lastTimestamp.append(QDateTime::currentDateTime().toString("dd.MM.yyyy_HH:mm"));
        text.append(_lastTimestamp);
        _ui->backupNameLineEdit->setText(text);
        _ui->backupNameLineEdit->setCursorPosition(0);
    }
    else
    {
        QString text = _ui->backupNameLineEdit->text();
        if(!_lastTimestamp.isEmpty() && !text.isEmpty())
        {
            int index = text.indexOf(_lastTimestamp, -(_lastTimestamp.count()));
            if(index != -1)
            {
                text.truncate(index);
                _ui->backupNameLineEdit->setText(text);
            }
        }
    }
}

void MainWindow::on_backupListInfoLabel_linkActivated(const QString &link)
{
    Q_UNUSED(link)
    // Can't select multiple directories and files at the same time using the Native dialog
    // Thus instead of being able to select only dirs or files, we'll be using a custom
    // Qt dialog for now
    /*
    QStringList paths = QFileDialog::getOpenFileNames(this,
                                                      tr("Select files and directories")
                                                      , QDir::homePath());
                                                      */
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog,true);
    QListView *l = dialog.findChild<QListView*>("listView");
    if(l)
        l->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView *t = dialog.findChild<QTreeView*>();
    if(t)
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    if(dialog.exec())
        QMetaObject::invokeMethod(_ui->backupListWidget, "addItemsWithUrls", Qt::QueuedConnection, Q_ARG(QList<QUrl>, dialog.selectedUrls()));
//    qDebug() << dialog.selectedUrls();
}

void MainWindow::on_backupButton_clicked()
{
    QList<QUrl> urls;

    for(int i = 0; i < _ui->backupListWidget->count(); ++i)
    {
        urls << dynamic_cast<BackupListItem*>(_ui->backupListWidget->item(i))->url();
    }

    BackupJobPtr job(new BackupJob);
    job->name = _ui->backupNameLineEdit->text();
    job->urls = urls;

    emit backupNow(job);
}

void MainWindow::updateInspectArchive()
{
    if(_currentArchiveDetail)
    {
        _ui->archiveNameLabel->setText(_currentArchiveDetail->name);
        _ui->archiveTotalSizeLabel->setText(QString::number(_currentArchiveDetail->sizeTotal));
        _ui->archiveTarsnapSizeLabel->setText(QString::number(_currentArchiveDetail->sizeUniqueCompressed));
        _ui->archiveDateLabel->setText(_currentArchiveDetail->timestamp.toString());
        _ui->archiveContentsLabel->setText(tr("Contents (%1)").arg(_currentArchiveDetail->contents.count()));
        _ui->archiveContentsPlainTextEdit->setPlainText(_currentArchiveDetail->contents.join('\n'));
    }
}

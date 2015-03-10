#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_archiveitemwidget.h"
#include "ui_backupitemwidget.h"
#include "backuplistitem.h"
#include "utils.h"
#include "debug.h"

#include <QPainter>
#include <QSettings>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QSharedPointer>
#include <QHostInfo>
#include <QMessageBox>
#include <QStandardPaths>

#define PURGE_SECONDS_DELAY 8

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::MainWindow),
    _loadingAnimation(":/resources/loading.gif"),
    _useSIPrefixes(false),
    _purgeTimerCount(0)
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

    _ui->mainContentSplitter->setCollapsible(0, false);
    _ui->journalLog->hide();
    _ui->loadingIconLabel->setMovie(&_loadingAnimation);
    _ui->archiveDetailsWidget->hide();

    connect(&Debug::instance(), SIGNAL(message(QString)), this , SLOT(appendToConsoleLog(QString))
            , Qt::QueuedConnection);

    loadSettings();

    // TODO: REMOVE
    Ui::ArchiveItemWidget restoreItemUi;
    for(int i = 0; i < 10; i++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        QWidget *widget = new QWidget;
        restoreItemUi.setupUi(widget);
        _ui->backupRestoreListWidget->insertItem(i, item);
        _ui->backupRestoreListWidget->setItemWidget(item, widget);
    }

    // Ui actions
    _ui->browseListWidget->addAction(_ui->actionRefresh);
    connect(_ui->actionRefresh, SIGNAL(triggered()), _ui->browseListWidget
            , SIGNAL(getArchivesList()), Qt::QueuedConnection);
    _ui->backupListWidget->addAction(_ui->actionClearList);
    connect(_ui->actionClearList, SIGNAL(triggered()), _ui->backupListWidget
            , SLOT(clear()), Qt::QueuedConnection);
    _ui->backupListWidget->addAction(_ui->actionBrowseItems);
    connect(_ui->actionBrowseItems, SIGNAL(triggered()), this, SLOT(browseForBackupItems()));
    connect(_ui->backupListInfoLabel, SIGNAL(linkActivated(QString)), this,
            SLOT(browseForBackupItems()));

    connect(&_purgeTimer, SIGNAL(timeout()), this, SLOT(purgeTimerFired()));

    // Settings page
    connect(_ui->accountUserLineEdit, SIGNAL(editingFinished()), this, SLOT(commitSettings()));
    connect(_ui->accountMachineLineEdit, SIGNAL(editingFinished()), this, SLOT(commitSettings()));
    connect(_ui->accountMachineKeyLineEdit, SIGNAL(editingFinished()), this, SLOT(commitSettings()));
    connect(_ui->tarsnapPathLineEdit, SIGNAL(editingFinished()), this, SLOT(commitSettings()));
    connect(_ui->tarsnapCacheLineEdit, SIGNAL(editingFinished()), this, SLOT(commitSettings()));
    connect(_ui->aggressiveNetworkingCheckBox, SIGNAL(toggled(bool)), this, SLOT(commitSettings()));
    connect(_ui->accountMachineKeyLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateMachineKeyPath()));
    connect(_ui->tarsnapPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateTarsnapPath()));
    connect(_ui->tarsnapCacheLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateTarsnapCache()));
    connect(_ui->siPrefixesCheckBox, SIGNAL(toggled(bool)), this, SLOT(commitSettings()));
    connect(_ui->preservePathsCheckBox, SIGNAL(toggled(bool)), this, SLOT(commitSettings()));
    connect(_ui->downloadsDirLineEdit, SIGNAL(editingFinished()), this, SLOT(commitSettings()));

    // Backup and Browse
    connect(_ui->backupListWidget, SIGNAL(itemTotals(qint64,qint64)), this
            , SLOT(updateBackupItemTotals(qint64, qint64)));
    connect(_ui->browseListWidget, SIGNAL(getArchivesList()), this, SIGNAL(getArchivesList()));
    connect(this, SIGNAL(archivesList(QList<ArchivePtr >))
            , _ui->browseListWidget, SLOT(addArchives(QList<ArchivePtr >)));
    connect(_ui->browseListWidget, SIGNAL(inspectArchive(ArchivePtr)), this
            , SLOT(displayInspectArchive(ArchivePtr)));
    connect(_ui->browseListWidget, SIGNAL(deleteArchives(QList<ArchivePtr>)), this
            , SIGNAL(deleteArchives(QList<ArchivePtr>)));
    connect(_ui->browseListWidget, SIGNAL(restoreArchive(ArchivePtr,ArchiveRestoreOptions)),
            this, SIGNAL(restoreArchive(ArchivePtr,ArchiveRestoreOptions)));
    connect(_ui->mainTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentPaneChanged(int)));

    //lambda slots to quickly update various UI components
    connect(_ui->browseListWidget, &BrowseListWidget::getArchivesList,
            [=](){updateStatusMessage(tr("Refreshing archives list..."));});
    connect(this, &MainWindow::archivesList,
            [=](){updateStatusMessage(tr("Refreshing archives list...done"));});
    connect(this, &MainWindow::loadArchiveStats,
            [=](const ArchivePtr archive){updateStatusMessage(tr("Fetching details for archive <i>%1</i>.").arg(archive->name));});
    connect(this, &MainWindow::loadArchiveContents,
            [=](const ArchivePtr archive){updateStatusMessage(tr("Fetching contents for archive <i>%1</i>.").arg(archive->name));});
    connect(_ui->browseListWidget, &BrowseListWidget::deleteArchives,
            [=](const QList<ArchivePtr> archives){archivesDeleted(archives,false);});
    connect(_ui->backupNameLineEdit, &QLineEdit::textChanged,
            [=](const QString text){
                if(text.isEmpty())
                    _ui->backupButton->setEnabled(false);
                else if(!_ui->backupDetailLabel->text().isEmpty())
                    _ui->backupButton->setEnabled(true);
            });
    connect(this, &MainWindow::restoreArchive,
            [=](const ArchivePtr archive){updateStatusMessage(tr("Restoring archive <i>%1</i>...").arg(archive->name));});
    connect(_ui->downloadsDirLineEdit, &QLineEdit::textChanged,
            [=](){
                QFileInfo file(_ui->downloadsDirLineEdit->text());
                if(file.exists() && file.isDir() && file.isWritable())
                    _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
                else
                    _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
            });
}

MainWindow::~MainWindow()
{
    commitSettings();
    delete _ui;
}

void MainWindow::loadSettings()
{
    QSettings settings;
    _ui->accountUserLineEdit->setText(settings.value("tarsnap/user", "").toString());
    _ui->accountMachineKeyLineEdit->setText(settings.value("tarsnap/key", "").toString());
    _ui->accountMachineLineEdit->setText(settings.value("tarsnap/machine", "").toString());
    _ui->tarsnapPathLineEdit->setText(settings.value("tarsnap/path", "").toString());
    _ui->tarsnapCacheLineEdit->setText(settings.value("tarsnap/cache", "").toString());
    _ui->aggressiveNetworkingCheckBox->setChecked(settings.value("tarsnap/aggressive_networking", false).toBool());
    _useSIPrefixes = settings.value("app/si_prefixes", false).toBool();
    _ui->siPrefixesCheckBox->setChecked(_useSIPrefixes);
    _ui->preservePathsCheckBox->setChecked(settings.value("tarsnap/preserve_pathnames", true).toBool());
    _ui->downloadsDirLineEdit->setText(settings.value("app/downloads_dir", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString());
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
        else if(_ui->journalLog->isVisible())
            _ui->expandJournalButton->toggle();
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
}

void MainWindow::backupJobUpdate(BackupJobPtr job)
{
    switch (job->status) {
    case JobStatus::Completed:
        updateStatusMessage(tr("Job <i>%1</i> completed. (%2 used on Tarsnap)")
                            .arg(job->name).arg(Utils::humanBytes(job->archive->sizeUniqueCompressed, _useSIPrefixes))
                            ,job->archive->archiveStats());
        break;
    case JobStatus::Started:
        updateStatusMessage(tr("Job <i>%1</i> created.").arg(job->name));
        break;
    case JobStatus::Running:
        updateStatusMessage(tr("Job <i>%1</i> is running.").arg(job->name));
        break;
    case JobStatus::Failed:
        updateStatusMessage(tr("Job <i>%1</i> failed: %2").arg(job->name).arg(job->output.simplified())
                           ,tr("%1").arg(job->output));
        break;
    case JobStatus::Paused:
        updateStatusMessage(tr("Job <i>%1</i> paused.").arg(job->name));
        break;
    default:
        break;
    }
}

void MainWindow::archivesDeleted(QList<ArchivePtr> archives, bool done)
{
    if(archives.count() > 1)
    {
        QString detail(archives[0]->name);
        for(int i = 1; i < archives.count(); ++i) {
            ArchivePtr archive = archives.at(i);
            detail.append(QString::fromLatin1(", ") + archive->name);
        }
        updateStatusMessage(tr("Deleting archive <i>%1</i> and %2 more archives...%3")
                            .arg(archives.first()->name).arg(archives.count()-1)
                            .arg(done?"done":""), detail);
    }
    else if(archives.count() == 1)
    {
        updateStatusMessage(tr("Deleting archive <i>%1</i>...%2").arg(archives.first()->name)
                            .arg(done?"done":""));
    }
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

void MainWindow::updateSettingsSummary(qint64 sizeTotal, qint64 sizeCompressed, qint64 sizeUniqueTotal, qint64 sizeUniqueCompressed, qint64 archiveCount, qreal credit, QString accountStatus)
{
    QString tooltip(tr("\t\tTotal size\tCompressed size\n"
                       "all archives\t%1\t\t%2\n"
                       "unique data\t%3\t\t%4").arg(sizeTotal).arg(sizeCompressed)
                       .arg(sizeUniqueTotal).arg(sizeUniqueCompressed));
    _ui->accountTotalSizeLabel->setText(Utils::humanBytes(sizeTotal, _useSIPrefixes));
    _ui->accountTotalSizeLabel->setToolTip(tooltip);
    _ui->accountActualSizeLabel->setText(Utils::humanBytes(sizeUniqueCompressed, _useSIPrefixes));
    _ui->accountActualSizeLabel->setToolTip(tooltip);
    _ui->accountStorageSavedLabel->setText(Utils::humanBytes(sizeTotal-sizeUniqueCompressed, _useSIPrefixes));
    _ui->accountStorageSavedLabel->setToolTip(tooltip);
    _ui->accountArchivesCountLabel->setText(QString::number(archiveCount));
    _ui->accountCreditLabel->setText(QString::number(credit, 'f'));
    _ui->accountStatusLabel->setText(accountStatus);
}

void MainWindow::repairCacheStatus(JobStatus status, QString reason)
{
    switch (status) {
    case JobStatus::Completed:
        updateStatusMessage(tr("Cache repair succeeded."), reason);
        break;
    case JobStatus::Failed:
    default:
        updateStatusMessage(tr("Cache repair failed. Hover mouse for details."), reason);
        break;
    }
}

void MainWindow::purgeArchivesStatus(JobStatus status, QString reason)
{
    switch (status) {
    case JobStatus::Completed:
        updateStatusMessage(tr("All archives purged successfully."), reason);
        break;
    case JobStatus::Failed:
    default:
        updateStatusMessage(tr("Archives purging failed. Hover mouse for details."), reason);
        break;
    }
}

void MainWindow::restoreArchiveStatus(ArchivePtr archive, JobStatus status, QString reason)
{
    switch (status) {
    case JobStatus::Completed:
        updateStatusMessage(tr("Restoring archive <i>%1</i>...done").arg(archive->name), reason);
        break;
    case JobStatus::Failed:
    default:
        updateStatusMessage(tr("Restoring archive <i>%1</i>...failed. Hover mouse for details.").arg(archive->name), reason);
        break;
    }
}

void MainWindow::updateBackupItemTotals(qint64 count, qint64 size)
{
    if(count != 0)
    {
        _ui->backupDetailLabel->setText(tr("%1 items (%2)").arg(count).arg(Utils::humanBytes(size, _useSIPrefixes)));
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
        _ui->archiveDateLabel->setText(_currentArchiveDetail->timestamp.toString());
        _ui->archiveTotalSizeLabel->setText(Utils::humanBytes(_currentArchiveDetail->sizeTotal, _useSIPrefixes));
        _ui->archiveTotalSizeLabel->setToolTip(_currentArchiveDetail->archiveStats());
        _ui->archiveTarsnapSizeLabel->setText(Utils::humanBytes(_currentArchiveDetail->sizeUniqueCompressed, _useSIPrefixes));
        _ui->archiveTarsnapSizeLabel->setToolTip(_currentArchiveDetail->archiveStats());
        _ui->archiveCommandLabel->setText(_currentArchiveDetail->command);
        int count = _currentArchiveDetail->contents.count();
        _ui->archiveContentsLabel->setText(tr("Contents (%1)").arg((count == 0) ? tr("loading..."):QString::number(count)));
        _ui->archiveContentsPlainTextEdit->setPlainText(_currentArchiveDetail->contents.join('\n'));
    }
}

void MainWindow::updateStatusMessage(QString message, QString detail)
{
    _ui->statusBarLabel->setText(message);
    _ui->statusBarLabel->setToolTip(detail);
    _ui->journalLog->appendHtml(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss")).arg(message));
}

void MainWindow::currentPaneChanged(int index)
{
    Q_UNUSED(index);
    if(_ui->mainTabWidget->currentWidget() == _ui->settingsTab)
    {
        emit getOverallStats();
    }
}

void MainWindow::commitSettings()
{
    DEBUG << "COMMIT SETTINGS";
    QSettings settings;
    settings.setValue("tarsnap/path",    _ui->tarsnapPathLineEdit->text());
    settings.setValue("tarsnap/cache",   _ui->tarsnapCacheLineEdit->text());
    settings.setValue("tarsnap/key",     _ui->accountMachineKeyLineEdit->text());
    settings.setValue("tarsnap/machine", _ui->accountMachineLineEdit->text());
    settings.setValue("tarsnap/user",    _ui->accountUserLineEdit->text());
    settings.setValue("tarsnap/aggressive_networking", _ui->aggressiveNetworkingCheckBox->isChecked());
    settings.setValue("tarsnap/preserve_pathnames", _ui->preservePathsCheckBox->isChecked());
    settings.setValue("app/si_prefixes", _ui->siPrefixesCheckBox->isChecked());
    settings.setValue("app/downloads_dir", _ui->downloadsDirLineEdit->text());
    settings.sync();
    emit settingsChanged();
}

void MainWindow::validateMachineKeyPath()
{
    QFileInfo machineKeyFile(_ui->accountMachineKeyLineEdit->text());
    if(machineKeyFile.exists() && machineKeyFile.isFile() && machineKeyFile.isReadable())
        _ui->accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: black;}");
    else
        _ui->accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: red;}");
}

void MainWindow::validateTarsnapPath()
{
    if(Utils::validateTarsnapPath(_ui->tarsnapPathLineEdit->text()).isEmpty())
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: red;}");
    else
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: black;}");
}

void MainWindow::validateTarsnapCache()
{
    if(Utils::validateTarsnapCache(_ui->tarsnapCacheLineEdit->text()).isEmpty())
        _ui->tarsnapCacheLineEdit->setStyleSheet("QLineEdit {color: red;}");
    else
        _ui->tarsnapCacheLineEdit->setStyleSheet("QLineEdit {color: black;}");
}

void MainWindow::purgeTimerFired()
{
    if(_purgeTimerCount <= 1)
    {
        _purgeTimer.stop();
        _ui->purgeArchivesButton->setText(tr("Purge archives"));
        _ui->purgeArchivesButton->setToolTip(tr("Nukes all archives"));
        updateStatusMessage(tr("Purging all archives..."));
        emit purgeArchives();
    }
    else
    {
        _ui->purgeArchivesButton->setText(tr("Purging all archives in %1 seconds..").arg(--_purgeTimerCount));
    }
}

void MainWindow::appendToConsoleLog(QString msg)
{
    _ui->consoleLogPlainTextEdit->appendPlainText(msg);
}

void MainWindow::browseForBackupItems()
{
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
}

void MainWindow::on_accountMachineUseHostnameButton_clicked()
{
    _ui->accountMachineLineEdit->setText(QHostInfo::localHostName());
    commitSettings();
}

void MainWindow::on_accountMachineKeyBrowseButton_clicked()
{
    QString key = QFileDialog::getOpenFileName(this, tr("Browse for existing machine key"));
    _ui->accountMachineKeyLineEdit->setText(key);
    commitSettings();
}

void MainWindow::on_tarsnapPathBrowseButton_clicked()
{
    QString tarsnapPath = QFileDialog::getExistingDirectory(this,
                                                            tr("Find Tarsnap client"),
                                                            "");
    _ui->tarsnapPathLineEdit->setText(tarsnapPath);
    commitSettings();
}

void MainWindow::on_tarsnapCacheBrowseButton_clicked()
{
    QString tarsnapCacheDir = QFileDialog::getExistingDirectory(this,
                                                            tr("Tarsnap cache location"),
                                                            "");
    _ui->tarsnapCacheLineEdit->setText(tarsnapCacheDir);
    commitSettings();
}

void MainWindow::on_repairCacheButton_clicked()
{
    emit repairCache();
}

void MainWindow::on_purgeArchivesButton_clicked()
{
    if(_purgeTimer.isActive())
    {
        _purgeTimer.stop();
        _ui->purgeArchivesButton->setText(tr("Purge archives"));
        _ui->purgeArchivesButton->setToolTip(tr("Nukes all archives"));
    }
    else
    {
        QMessageBox::StandardButton confirm = QMessageBox::question(this, tr("Confirm action")
                                                                    ,tr("Are you sure you want to nuke all archives stored for this key?\nWarning: This action cannot be undone. All archives will be lost forever.")
                                                                    ,( QMessageBox::Yes | QMessageBox::No ), QMessageBox::No);
        if(confirm == QMessageBox::Yes)
        {
            _purgeTimerCount = PURGE_SECONDS_DELAY;
            _ui->purgeArchivesButton->setText(tr("Purging all archives in %1 seconds...").arg(_purgeTimerCount));
            _ui->purgeArchivesButton->setToolTip(tr("Press to Cancel"));
            _purgeTimer.start(1000);
        }
    }
}

void MainWindow::on_runSetupWizard_clicked()
{
    QMessageBox::StandardButton confirm = QMessageBox::question(this, tr("Confirm action")
                                                                ,tr("Reset current settings and run the setup wizard?")
                                                                ,( QMessageBox::Yes | QMessageBox::No ), QMessageBox::No);
    if(confirm == QMessageBox::Yes)
    {
        QSettings settings;
        settings.clear();
        settings.sync();
        emit runSetupWizard();
    }
}

void MainWindow::on_expandJournalButton_toggled(bool checked)
{
    if(checked)
        _ui->journalLog->show();
    else
        _ui->journalLog->hide();
}

void MainWindow::on_downloadsDirBrowseButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Browse for directory"),
                                                     QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    if(!path.isEmpty())
        _ui->downloadsDirLineEdit->setText(path);
}

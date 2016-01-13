#include "mainwindow.h"
#include "backuplistitem.h"
#include "debug.h"
#include "filepickerdialog.h"
#include "ui_aboutwidget.h"
#include "ui_archiveitemwidget.h"
#include "ui_backupitemwidget.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QHostInfo>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <QSharedPointer>
#include <QShortcut>
#include <QStandardPaths>

#define PURGE_SECONDS_DELAY 8

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::MainWindow),
      _logo(":/icons/tarsnap-logo.png"),
      _icon(":/icons/tarsnap-logo.png"),
      _useSIPrefixes(false),
      _purgeTimerCount(0),
      _purgeCountdownWindow(this),
      _tarsnapAccount(this)
{
    connect(&Debug::instance(), &Debug::message, this, [&](const QString &msg)
            {_ui->consoleLog->appendPlainText(msg);});
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
    _ui->setupUi(this);
    _ui->backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->jobListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
#ifdef Q_OS_OSX
    new QShortcut(QKeySequence("Ctrl+M"), this, SLOT(showMinimized()));
#endif
    new QShortcut(QKeySequence("Ctrl+K"), this, SIGNAL(getTaskInfo()));

    loadSettings();

    // About menu action and widget initialization
    Ui::aboutWidget aboutUi;
    aboutUi.setupUi(&_aboutWindow);
    aboutUi.versionLabel->setText(tr("version ") +
                                  QCoreApplication::applicationVersion());
    _aboutWindow.setWindowFlags((_aboutWindow.windowFlags() |
                                 Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
    connect(aboutUi.checkUpdateButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/Tarsnap/tarsnap-gui/releases"));
    });

    QMenuBar menuBar;
    if(menuBar.isNativeMenuBar())
    {
        QAction *actionAbout = new QAction(this);
        actionAbout->setMenuRole(QAction::AboutRole);
        connect(actionAbout, &QAction::triggered, &_aboutWindow, &QDialog::show);
        QMenu *appMenu = new QMenu(this);
        appMenu->addAction(actionAbout);
        menuBar.addMenu(appMenu);
    }
    connect(_ui->aboutButton, &QPushButton::clicked, &_aboutWindow,
            &QDialog::show);
    // --

    _ui->mainTabWidget->setCurrentWidget(_ui->backupTab);
    _ui->settingsToolbox->setCurrentWidget(_ui->settingsAccountPage);
    _ui->mainContentSplitter->setCollapsible(0, false);
    _ui->journalLog->hide();
    _ui->archiveDetailsWidget->hide();
    _ui->jobDetailsWidget->hide();
    _ui->outOfDateNoticeLabel->hide();
#if(QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    _ui->consoleLog->setPlaceholderText(tr("No events yet"));
    _ui->journalLog->setPlaceholderText(tr("No events yet"));
#endif

    // Purge widget setup
    _purgeCountdownWindow.setIcon(QMessageBox::Critical);
    _purgeCountdownWindow.setWindowTitle(tr("Deleting all archives: press Cancel to abort"));
    _purgeCountdownWindow.setStandardButtons(QMessageBox::Cancel);
    connect(&_purgeTimer, &QTimer::timeout, this, &MainWindow::purgeTimerFired);
    // --

    // Ui actions setup
    _ui->archiveListWidget->addAction(_ui->actionRefresh);
    connect(_ui->actionRefresh, &QAction::triggered, this,
            &MainWindow::loadArchives);
    _ui->backupListWidget->addAction(_ui->actionClearList);
    connect(_ui->actionClearList, &QAction::triggered, _ui->backupListWidget,
            &BackupListWidget::clear);
    _ui->backupListWidget->addAction(_ui->actionBrowseItems);
    connect(_ui->actionBrowseItems, &QAction::triggered, this,
            &MainWindow::browseForBackupItems);
    _ui->settingsTab->addAction(_ui->actionRefreshAccount);
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            &MainWindow::getOverallStats);
    connect(_ui->actionRefreshAccount, &QAction::triggered, &_tarsnapAccount,
            &TarsnapAccount::getAccountCredit);
    this->addAction(_ui->actionGoBackup);
    this->addAction(_ui->actionGoBrowse);
    this->addAction(_ui->actionGoJobs);
    this->addAction(_ui->actionGoSettings);
    this->addAction(_ui->actionGoHelp);
    this->addAction(_ui->actionShowJournal);
    connect(_ui->actionGoBackup, &QAction::triggered,
            [&]() { _ui->mainTabWidget->setCurrentWidget(_ui->backupTab); });
    connect(_ui->actionGoBrowse, &QAction::triggered,
            [&]() { _ui->mainTabWidget->setCurrentWidget(_ui->archivesTab); });
    connect(_ui->actionGoJobs, &QAction::triggered,
            [&]() { _ui->mainTabWidget->setCurrentWidget(_ui->jobsTab); });
    connect(_ui->actionGoSettings, &QAction::triggered,
            [&]() { _ui->mainTabWidget->setCurrentWidget(_ui->settingsTab); });
    connect(_ui->actionGoHelp, &QAction::triggered,
            [&]() { _ui->mainTabWidget->setCurrentWidget(_ui->helpTab); });
    connect(_ui->actionShowJournal, &QAction::triggered,
            _ui->expandJournalButton, &QToolButton::click);
    connect(_ui->backupListInfoLabel, &QLabel::linkActivated, this,
            &MainWindow::browseForBackupItems);
    connect(_ui->backupButton, &QPushButton::clicked, this,
            &MainWindow::backupButtonClicked);
    connect(_ui->appendTimestampCheckBox, &QCheckBox::toggled, this,
            &MainWindow::appendTimestampCheckBoxToggled);
    connect(_ui->accountMachineUseHostnameButton, &QPushButton::clicked, this,
            &MainWindow::accountMachineUseHostnameButtonClicked);
    connect(_ui->accountMachineKeyBrowseButton, &QPushButton::clicked, this,
            &MainWindow::accountMachineKeyBrowseButtonClicked);
    connect(_ui->tarsnapPathBrowseButton, &QPushButton::clicked, this,
            &MainWindow::tarsnapPathBrowseButtonClicked);
    connect(_ui->tarsnapCacheBrowseButton, &QPushButton::clicked, this,
            &MainWindow::tarsnapCacheBrowseButton);
    connect(_ui->appDataDirBrowseButton, &QPushButton::clicked, this,
            &MainWindow::appDataButtonClicked);
    connect(_ui->purgeArchivesButton, &QPushButton::clicked, this,
            &MainWindow::purgeArchivesButtonClicked);
    connect(_ui->runSetupWizard, &QPushButton::clicked, this,
            &MainWindow::runSetupWizardClicked);
    connect(_ui->expandJournalButton, &QToolButton::toggled, this,
            &MainWindow::expandJournalButtonToggled);
    connect(_ui->downloadsDirBrowseButton, &QPushButton::clicked, this,
            &MainWindow::downloadsDirBrowseButtonClicked);
    connect(_ui->busyWidget, &BusyWidget::clicked, this,
            &MainWindow::getTaskInfo);

    // Settings page
    connect(_ui->accountUserLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->accountMachineLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->accountMachineKeyLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->tarsnapPathLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->tarsnapCacheLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->aggressiveNetworkingCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->accountMachineKeyLineEdit, &QLineEdit::textChanged, this,
            &MainWindow::validateMachineKeyPath);
    connect(_ui->tarsnapPathLineEdit, &QLineEdit::textChanged, this,
            &MainWindow::validateTarsnapPath);
    connect(_ui->tarsnapCacheLineEdit, &QLineEdit::textChanged, this,
            &MainWindow::validateTarsnapCache);
    connect(_ui->siPrefixesCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->notificationsCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->preservePathsCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->downloadsDirLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->traverseMountCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->followSymLinksCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->skipFilesSizeSpinBox, &QSpinBox::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->skipSystemJunkCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->skipSystemLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->skipNoDumpCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->simulationCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->ignoreConfigCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui->skipSystemDefaultsButton, &QPushButton::clicked,
            [&]() { _ui->skipSystemLineEdit->setText(DEFAULT_SKIP_FILES); });
    connect(&_tarsnapAccount, &TarsnapAccount::accountCredit, this,
            &MainWindow::updateAccountCredit);
    connect(_ui->updateCreditButton, &QPushButton::clicked,
            _ui->actionRefreshAccount, &QAction::trigger);
    _ui->machineActivity->hide();
    _ui->machineActivityLabel->hide();
    _ui->machineActivityShowButton->hide();
    _ui->formLayout_3->removeItem(_ui->machineActivityHorizontalLayout);
    // ---

    // Backup and Archives
    connect(_ui->backupListWidget, &BackupListWidget::itemTotals, this,
            &MainWindow::updateBackupItemTotals);
    connect(this, &MainWindow::archiveList, _ui->archiveListWidget,
            &ArchiveListWidget::addArchives);
    connect(_ui->archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui->archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::displayJobDetails,
            _ui->jobListWidget, &JobListWidget::selectJobByRef);
    connect(_ui->archiveJobLabel, &TextLabel::clicked, [&]() {
        _ui->jobListWidget->selectJobByRef(_currentArchiveDetail->jobRef());
    });

    // Jobs
    connect(_ui->addJobButton, &QToolButton::clicked, this,
            &MainWindow::addJobClicked);
    connect(_ui->jobDetailsWidget, &JobWidget::cancel, this,
            &MainWindow::hideJobDetails);
    connect(_ui->jobDetailsWidget, &JobWidget::jobAdded, _ui->jobListWidget,
            &JobListWidget::addJob);
    connect(_ui->jobDetailsWidget, &JobWidget::jobAdded, _ui->jobListWidget,
            &JobListWidget::selectJob);
    connect(_ui->jobDetailsWidget, &JobWidget::inspectJobArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_ui->jobDetailsWidget, &JobWidget::restoreJobArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui->jobDetailsWidget, &JobWidget::deleteJobArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui->jobDetailsWidget, &JobWidget::enableSave, _ui->addJobButton,
            &QToolButton::setEnabled);
    connect(_ui->jobListWidget, &JobListWidget::displayJobDetails, this,
            &MainWindow::displayJobDetails);
    connect(_ui->jobListWidget, &JobListWidget::backupJob, this,
            &MainWindow::backupNow);
    connect(_ui->jobListWidget, &JobListWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui->jobListWidget, &JobListWidget::deleteJob, this,
            &MainWindow::deleteJob);
    connect(this, &MainWindow::jobsList, _ui->jobListWidget,
            &JobListWidget::addJobs);
// Disabled functionality
//    connect(&_tarsnapAccount, SIGNAL(lastMachineActivity(QStringList)), this, SLOT(updateLastMachineActivity(QStringList)));
//    connect(_ui->accountActivityShowButton, &QPushButton::clicked,
//    [&]() {
//        _tarsnapAccount.getAccountInfo(true, false);
//    });
//    connect(_ui->machineActivityShowButton, &QPushButton::clicked,
//    [&]() {
//        _tarsnapAccount.getAccountInfo(false, true);
//    });

    _ui->jobListWidget->addAction(_ui->actionAddJob);
    connect(_ui->actionAddJob, &QAction::triggered, this,
            &MainWindow::addJobClicked);
    QMenu *addJobMenu = new QMenu(_ui->addJobButton);
    addJobMenu->addAction(_ui->actionBackupAllJobs);
    connect(_ui->actionBackupAllJobs, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::backupAllJobs);
    _ui->addJobButton->setMenu(addJobMenu);
    _ui->jobListWidget->addAction(_ui->actionJobBackup);
    connect(_ui->actionJobBackup, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::backupSelectedItems);

    // lambda slots to quickly update various UI components
    connect(this, &MainWindow::loadArchives, [&]() {
        updateStatusMessage(tr("Updating archives list from remote..."));
    });
    connect(this, &MainWindow::archiveList,
            [&](const QList<ArchivePtr> archives, bool fromRemote) {
                Q_UNUSED(archives);
                if(fromRemote)
                    updateStatusMessage(
                        tr("Updating archives list from remote...done"));
            });
    connect(this, &MainWindow::loadArchiveStats, [&](const ArchivePtr archive) {
        updateStatusMessage(
            tr("Fetching details for archive <i>%1</i>.").arg(archive->name()));
    });
    connect(this, &MainWindow::loadArchiveContents, [&](const ArchivePtr archive) {
        updateStatusMessage(
            tr("Fetching contents for archive <i>%1</i>.").arg(archive->name()));
    });
    connect(_ui->archiveListWidget, &ArchiveListWidget::deleteArchives,
            [&](const QList<ArchivePtr> archives) {
                notifyArchivesDeleted(archives, false);
            });
    connect(_ui->jobDetailsWidget, &JobWidget::deleteJobArchives,
            [&](const QList<ArchivePtr> archives) {
                notifyArchivesDeleted(archives, false);
            });
    connect(_ui->backupNameLineEdit, &QLineEdit::textChanged,
            [&](const QString text) {
                if(text.isEmpty())
                {
                    _ui->backupButton->setEnabled(false);
                    _ui->appendTimestampCheckBox->setChecked(false);
                }
                else if(_ui->backupListWidget->count())
                {
                    _ui->backupButton->setEnabled(true);
                }
            });
    connect(this, &MainWindow::restoreArchive, [&](const ArchivePtr archive) {
        updateStatusMessage(
            tr("Restoring archive <i>%1</i>...").arg(archive->name()));
    });
    connect(_ui->downloadsDirLineEdit, &QLineEdit::textChanged, [&]() {
        QFileInfo file(_ui->downloadsDirLineEdit->text());
        if(file.exists() && file.isDir() && file.isWritable())
            _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
        else
            _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
    });
    connect(_ui->jobListWidget, &JobListWidget::backupJob,
            [&](BackupTaskPtr backup) {
                connect(backup, &BackupTask::statusUpdate, this,
                        &MainWindow::backupTaskUpdate, QUEUED);
            });
    connect(_ui->jobListWidget, &JobListWidget::deleteJob,
            [&](JobPtr job, bool purgeArchives) {
                if(purgeArchives)
                    updateStatusMessage(tr("Job <i>%1</i> deleted. Deleting %2 "
                                           "associated archives next...")
                                            .arg(job->name())
                                            .arg(job->archives().count()));
                else
                    updateStatusMessage(
                        tr("Job <i>%1</i> deleted.").arg(job->name()));
            });
    connect(_ui->jobDetailsWidget, &JobWidget::jobAdded, [&](JobPtr job) {
        emit jobAdded(job);
        updateStatusMessage(tr("Job <i>%1</i> added.").arg(job->name()));
    });
    connect(_ui->statusBarLabel, &TextLabel::clicked, [&]() {
        _ui->expandJournalButton->setChecked(!_ui->expandJournalButton->isChecked());
    });
    connect(_ui->simulationCheckBox, &QCheckBox::stateChanged, [&](int state) {
        if(state == Qt::Unchecked)
        {
            emit loadArchives();
            _ui->simulationIcon->hide();
        }
        else
        {
            _ui->simulationIcon->show();
        }
    });
    connect(_ui->repairCacheButton, &QPushButton::clicked, this,
            [&](){ emit repairCache(true); });
}

MainWindow::~MainWindow()
{
    commitSettings();
    delete _ui;
}

void MainWindow::loadSettings()
{
    QSettings settings;

    _ui->accountCreditLabel->setText(
        settings.value("tarsnap/credit", tr("unknown")).toString());
    QDate creditDate = settings.value("tarsnap/credit_date", QDate()).toDate();
    if(creditDate.isValid())
    {
        _ui->accountCreditLabel->setToolTip(creditDate.toString());
        qint32 daysElapsed = creditDate.daysTo(QDate::currentDate());
        if(daysElapsed > 10)
        {
            _ui->outOfDateNoticeLabel->setText(
                _ui->outOfDateNoticeLabel->text().arg(daysElapsed));
            _ui->outOfDateNoticeLabel->show();
        }
        else
        {
            _ui->outOfDateNoticeLabel->hide();
        }
    }
    else
    {
        _ui->accountCreditLabel->setToolTip(
            tr("This info is updated on demand. Press the big Tarsnap button "
               "above to update."));
    }

    _ui->machineActivity->setText(
        settings.value("tarsnap/machine_activity", tr("unknown")).toString());
    _ui->accountUserLineEdit->setText(
        settings.value("tarsnap/user", "").toString());
    _ui->accountMachineKeyLineEdit->setText(
        settings.value("tarsnap/key", "").toString());
    _ui->accountMachineLineEdit->setText(
        settings.value("tarsnap/machine", "").toString());
    _ui->tarsnapPathLineEdit->setText(
        settings.value("tarsnap/path", "").toString());
    _tarsnapVersion = settings.value("tarsnap/version", "").toString();
    setTarsnapVersion(_tarsnapVersion);
    _ui->tarsnapCacheLineEdit->setText(
        settings.value("tarsnap/cache", "").toString());
    _ui->aggressiveNetworkingCheckBox->setChecked(
        settings.value("tarsnap/aggressive_networking", false).toBool());
    _ui->traverseMountCheckBox->setChecked(
        settings.value("tarsnap/traverse_mount", true).toBool());
    _ui->followSymLinksCheckBox->setChecked(
        settings.value("tarsnap/follow_symlinks", false).toBool());
    _ui->preservePathsCheckBox->setChecked(
        settings.value("tarsnap/preserve_pathnames", true).toBool());
    _ui->ignoreConfigCheckBox->setChecked(
        settings.value("tarsnap/no_default_config", false).toBool());
    _ui->simulationCheckBox->setChecked(
        settings.value("tarsnap/dry_run", false).toBool());
    _ui->simulationIcon->setVisible(_ui->simulationCheckBox->isChecked());

    _useSIPrefixes = settings.value("app/si_prefixes", false).toBool();
    _ui->siPrefixesCheckBox->setChecked(_useSIPrefixes);
    _ui->skipFilesSizeSpinBox->setValue(
        settings.value("app/skip_files_size", 0).toULongLong());
    _ui->skipSystemJunkCheckBox->setChecked(
        settings.value("app/skip_system_enabled", false).toBool());
    _ui->skipSystemLineEdit->setEnabled(_ui->skipSystemJunkCheckBox->isChecked());
    _ui->skipSystemLineEdit->setText(
        settings.value("app/skip_system_files", DEFAULT_SKIP_FILES).toString());
    _ui->skipNoDumpCheckBox->setChecked(
        settings.value("app/skip_nodump", false).toBool());
    _ui->downloadsDirLineEdit->setText(
        settings.value("app/downloads_dir",
                       QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
        .toString());
    _ui->appDataDirLineEdit->setText(
        settings.value("app/app_data", "").toString());
    _ui->notificationsCheckBox->setChecked(
        settings.value("app/notifications", true).toBool());
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    _icon.paint(&p, width() - _logo.width() - 5, 2, _logo.width(),
                _logo.height());
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
        _windowDragPos = event->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        QPoint diff   = event->pos() - _windowDragPos;
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
        if((_ui->mainTabWidget->currentWidget() == _ui->archivesTab) &&
           (_ui->archiveDetailsWidget->isVisible()))
        {
            _ui->archiveContentsPlainTextEdit->clear(); // reduce memory usage
            _ui->archiveDetailsWidget->hide();
        }
        if((_ui->mainTabWidget->currentWidget() == _ui->jobsTab) &&
           (_ui->jobDetailsWidget->isVisible()))
        {
            hideJobDetails();
        }
        else if(_ui->journalLog->isVisible())
        {
            _ui->expandJournalButton->toggle();
        }
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
}

void MainWindow::backupTaskUpdate(const TaskStatus &status)
{
    BackupTaskPtr backupTask = qobject_cast<BackupTaskPtr>(sender());
    switch(status)
    {
    case TaskStatus::Completed:
    {
        QString msg = tr("Backup <i>%1</i> completed. (%2 new data on Tarsnap)")
                      .arg(backupTask->name())
                      .arg(Utils::humanBytes(backupTask->archive()->sizeUniqueCompressed(),
                                             _useSIPrefixes));
        updateStatusMessage(msg, backupTask->archive()->archiveStats());
        emit displayNotification(msg.remove(QRegExp("<[^>]*>")));
        delete backupTask;
        break;
    }
    case TaskStatus::Queued:
        updateStatusMessage(
            tr("Backup <i>%1</i> queued.").arg(backupTask->name()));
        break;
    case TaskStatus::Running:
        updateStatusMessage(
            tr("Backup <i>%1</i> is running.").arg(backupTask->name()));
        break;
    case TaskStatus::Failed:
    {
        QString msg = tr("Backup <i>%1</i> failed: %2")
                          .arg(backupTask->name())
                          .arg(backupTask->output().simplified());
        updateStatusMessage(msg, backupTask->output());
        emit displayNotification(msg.remove(QRegExp("<[^>]*>")));
        delete backupTask;
        break;
    }
    case TaskStatus::Paused:
        updateStatusMessage(
            tr("Backup <i>%1</i> paused.").arg(backupTask->name()));
        break;
    default:
        break;
    }
}

void MainWindow::notifyArchivesDeleted(QList<ArchivePtr> archives, bool done)
{
    if(archives.count() > 1)
    {
        QString detail(archives[0]->name());
        for(int i = 1; i < archives.count(); ++i)
        {
            ArchivePtr archive = archives.at(i);
            detail.append(QString::fromLatin1(", ") + archive->name());
        }
        updateStatusMessage(
            tr("Deleting archive <i>%1</i> and %2 more archives...%3")
                .arg(archives.first()->name())
                .arg(archives.count() - 1)
                .arg(done ? "done" : ""),
            detail);
    }
    else if(archives.count() == 1)
    {
        updateStatusMessage(tr("Deleting archive <i>%1</i>...%2")
                                .arg(archives.first()->name())
                                .arg(done ? "done" : ""));
    }
}

void MainWindow::archivesDeleted(QList<ArchivePtr> archives)
{
    notifyArchivesDeleted(archives, true);
}

void MainWindow::updateLoadingAnimation(bool idle)
{
    if(idle)
        _ui->busyWidget->stop();
    else
        _ui->busyWidget->animate();
}

void MainWindow::updateSettingsSummary(quint64 sizeTotal, quint64 sizeCompressed,
                                       quint64 sizeUniqueTotal,
                                       quint64 sizeUniqueCompressed,
                                       quint64 archiveCount)
{
    QString tooltip(tr("\t\tTotal size\tCompressed size\n"
                       "all archives\t%1\t\t%2\n"
                       "unique data\t%3\t\t%4")
                        .arg(sizeTotal)
                        .arg(sizeCompressed)
                        .arg(sizeUniqueTotal)
                        .arg(sizeUniqueCompressed));
    _ui->accountTotalSizeLabel->setText(
        Utils::humanBytes(sizeTotal, _useSIPrefixes));
    _ui->accountTotalSizeLabel->setToolTip(tooltip);
    _ui->accountActualSizeLabel->setText(
        Utils::humanBytes(sizeUniqueCompressed, _useSIPrefixes));
    _ui->accountActualSizeLabel->setToolTip(tooltip);
    quint64 storageSaved = sizeTotal >= sizeUniqueCompressed ?
                           sizeTotal - sizeUniqueCompressed : 0;
    _ui->accountStorageSavedLabel->setText(Utils::humanBytes(storageSaved,
                                                             _useSIPrefixes));
    _ui->accountStorageSavedLabel->setToolTip(tooltip);
    _ui->accountArchivesCountLabel->setText(QString::number(archiveCount));
}

void MainWindow::repairCacheStatus(TaskStatus status, QString reason)
{
    switch(status)
    {
    case TaskStatus::Completed:
        updateStatusMessage(tr("Cache repair succeeded."), reason);
        break;
    case TaskStatus::Failed:
    default:
        updateStatusMessage(tr("Cache repair failed. Hover mouse for details."),
                            reason);
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Cache repair failed. It might be worth trying"
                                 " the 'Repair cache' button in Settings -> "
                                 " Advanced."));
        break;
    }
}

void MainWindow::purgeArchivesStatus(TaskStatus status, QString reason)
{
    switch(status)
    {
    case TaskStatus::Completed:
        updateStatusMessage(tr("All archives purged successfully."), reason);
        break;
    case TaskStatus::Failed:
    default:
        updateStatusMessage(
            tr("Archives purging failed. Hover mouse for details."), reason);
        break;
    }
}

void MainWindow::restoreArchiveStatus(ArchivePtr archive, TaskStatus status,
                                      QString reason)
{
    switch(status)
    {
    case TaskStatus::Completed:
        updateStatusMessage(
            tr("Restoring archive <i>%1</i>...done").arg(archive->name()),
            reason);
        break;
    case TaskStatus::Failed:
    default:
        updateStatusMessage(
            tr("Restoring archive <i>%1</i>...failed. Hover mouse for details.")
                .arg(archive->name()),
            reason);
        break;
    }
}

void MainWindow::setTarsnapVersion(QString versionString)
{
    _tarsnapVersion = versionString;
    if(_tarsnapVersion.isEmpty())
    {
        _ui->clientVersionLabel->clear();
        _ui->clientVersionLabel->hide();
    }
    else
    {
        _ui->clientVersionLabel->setText(tr("Tarsnap version ") +
                                         _tarsnapVersion + tr(" detected"));
        _ui->clientVersionLabel->show();
    }
}

void MainWindow::notificationRaise()
{
    raise();
    activateWindow();
    showNormal();
}

void MainWindow::updateBackupItemTotals(quint64 count, quint64 size)
{
    if(count != 0)
    {
        _ui->backupDetailLabel->setText(tr("%1 %2 (%3)").arg(count)
                                        .arg(count == 1 ? "item" : "items")
                                        .arg(Utils::humanBytes(size, _useSIPrefixes)));
        if(!_ui->backupNameLineEdit->text().isEmpty())
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
        disconnect(_currentArchiveDetail.data(), &Archive::changed, this,
                   &MainWindow::updateInspectArchive);

    _currentArchiveDetail = archive;

    if(_currentArchiveDetail)
        connect(_currentArchiveDetail.data(), &Archive::changed, this,
                &MainWindow::updateInspectArchive, QUEUED);

    if(archive->sizeTotal() == 0)
        emit loadArchiveStats(archive);

    if(archive->contents().count() == 0)
        emit loadArchiveContents(archive);

    updateInspectArchive();

    if(!_ui->archiveDetailsWidget->isVisible())
        _ui->archiveDetailsWidget->show();

    if(_ui->mainTabWidget->currentWidget() != _ui->archivesTab)
        _ui->mainTabWidget->setCurrentWidget(_ui->archivesTab);

    _ui->archiveListWidget->setSelectedArchive(archive);
}

void MainWindow::appendTimestampCheckBoxToggled(bool checked)
{
    if(checked)
    {
        QString text = _ui->backupNameLineEdit->text();
        _lastTimestamp.clear();
        _lastTimestamp.append("_");
        _lastTimestamp.append(
            QDateTime::currentDateTime().toString("yyyy-MM-dd-HH:mm:ss"));
        text.append(_lastTimestamp);
        _ui->backupNameLineEdit->setText(text);
        _ui->backupNameLineEdit->setCursorPosition(0);
    }
    else
    {
        QString text = _ui->backupNameLineEdit->text();
        if(!_lastTimestamp.isEmpty() && text.endsWith(_lastTimestamp))
        {
            text.chop(_lastTimestamp.length());
            _ui->backupNameLineEdit->setText(text);
        }
    }
}

void MainWindow::backupButtonClicked()
{
    QList<QUrl> urls;
    for(int i = 0; i < _ui->backupListWidget->count(); ++i)
        urls << static_cast<BackupListItem *>(_ui->backupListWidget->item(i))->url();

    BackupTaskPtr backup(new BackupTask);
    backup->setName(_ui->backupNameLineEdit->text());
    backup->setUrls(urls);
    backup->setOptionDryRun(_ui->simulationCheckBox->isChecked());
    backup->setOptionSkipNoDump(_ui->skipNoDumpCheckBox->isChecked());
    connect(backup, &BackupTask::statusUpdate, this,
            &MainWindow::backupTaskUpdate, QUEUED);
    emit backupNow(backup);
    _ui->appendTimestampCheckBox->setChecked(false);
}

void MainWindow::updateInspectArchive()
{
    if(_currentArchiveDetail)
    {
        _ui->archiveNameLabel->setText(_currentArchiveDetail->name());
        _ui->archiveDateLabel->setText(
            _currentArchiveDetail->timestamp().toString(Qt::DefaultLocaleLongDate));
        if(_currentArchiveDetail->jobRef().isEmpty())
        {
            _ui->archiveJobLabel->hide();
            _ui->archiveJobLabelField->hide();
        }
        else
        {
            _ui->archiveJobLabel->show();
            _ui->archiveJobLabelField->show();
            _ui->archiveJobLabel->setText(_currentArchiveDetail->jobRef());
        }
        _ui->archiveSizeLabel->setText(
            Utils::humanBytes(_currentArchiveDetail->sizeTotal(), _useSIPrefixes));
        _ui->archiveSizeLabel->setToolTip(_currentArchiveDetail->archiveStats());
        _ui->archiveUniqueDataLabel->setText(
            Utils::humanBytes(_currentArchiveDetail->sizeUniqueCompressed(),
                              _useSIPrefixes));
        _ui->archiveUniqueDataLabel->setToolTip(
            _currentArchiveDetail->archiveStats());
        _ui->archiveCommandLabel->setText(_currentArchiveDetail->command());
        QString contents = _currentArchiveDetail->contents();
        _ui->archiveContentsLabel->setText(tr("Contents (%1)")
                                           .arg(QString::number(contents.count('\n'))));
        _ui->archiveContentsPlainTextEdit->setPlainText(contents);
    }
}

void MainWindow::updateStatusMessage(QString message, QString detail)
{
    _ui->statusBarLabel->setText(message);
    _ui->statusBarLabel->setToolTip(detail);
    emit logMessage(message.remove(QRegExp("<[^>]*>"))); // remove html tags
}

void MainWindow::commitSettings()
{
    DEBUG << "COMMIT SETTINGS";
    QSettings settings;
    settings.setValue("tarsnap/path",    _ui->tarsnapPathLineEdit->text());
    settings.setValue("tarsnap/version", _tarsnapVersion);
    settings.setValue("tarsnap/cache",   _ui->tarsnapCacheLineEdit->text());
    settings.setValue("tarsnap/key",     _ui->accountMachineKeyLineEdit->text());
    settings.setValue("tarsnap/machine", _ui->accountMachineLineEdit->text());
    settings.setValue("tarsnap/user",    _ui->accountUserLineEdit->text());
    settings.setValue("tarsnap/aggressive_networking", _ui->aggressiveNetworkingCheckBox->isChecked());
    settings.setValue("tarsnap/preserve_pathnames", _ui->preservePathsCheckBox->isChecked());
    settings.setValue("tarsnap/traverse_mount", _ui->traverseMountCheckBox->isChecked());
    settings.setValue("tarsnap/follow_symlinks", _ui->followSymLinksCheckBox->isChecked());
    settings.setValue("tarsnap/no_default_config", _ui->ignoreConfigCheckBox->isChecked());
    settings.setValue("tarsnap/dry_run", _ui->simulationCheckBox->isChecked());
    settings.setValue("app/si_prefixes", _ui->siPrefixesCheckBox->isChecked());
    settings.setValue("app/skip_files_size", _ui->skipFilesSizeSpinBox->value());
    settings.setValue("app/skip_system_enabled", _ui->skipSystemJunkCheckBox->isChecked());
    settings.setValue("app/skip_system_files", _ui->skipSystemLineEdit->text());
    settings.setValue("app/skip_nodump", _ui->skipNoDumpCheckBox->isChecked());
    settings.setValue("app/downloads_dir", _ui->downloadsDirLineEdit->text());
    settings.setValue("app/app_data", _ui->appDataDirLineEdit->text());
    settings.setValue("app/notifications", _ui->notificationsCheckBox->isChecked());
    settings.sync();
    emit settingsChanged();
}

void MainWindow::validateMachineKeyPath()
{
    QFileInfo machineKeyFile(_ui->accountMachineKeyLineEdit->text());
    if(machineKeyFile.exists() && machineKeyFile.isFile() &&
       machineKeyFile.isReadable())
        _ui->accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: black;}");
    else
        _ui->accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: red;}");
}

void MainWindow::validateTarsnapPath()
{
    if(Utils::findTarsnapClientInPath(_ui->tarsnapPathLineEdit->text()).isEmpty())
    {
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: red;}");
        setTarsnapVersion("");
    }
    else
    {
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: black;}");
        emit getTarsnapVersion(_ui->tarsnapPathLineEdit->text());
    }
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
        _purgeCountdownWindow.accept();
        emit purgeArchives();
        updateStatusMessage(tr("Archives purge initiated..."));
    }
    else
    {
        --_purgeTimerCount;
        _purgeCountdownWindow.setText(
            tr("Purging all archives in %1 seconds...").arg(_purgeTimerCount));
    }
}

void MainWindow::appendToJournalLog(QDateTime timestamp, QString message)
{
    QTextCursor cursor(_ui->journalLog->document());
    if(!_ui->journalLog->document()->isEmpty())
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertBlock();
        cursor.movePosition(QTextCursor::NextBlock);
    }
    QColor bgcolor;
    int    blockCount = _ui->journalLog->document()->blockCount();
    if(blockCount % 2)
        bgcolor = qApp->palette().base().color();
    else
        bgcolor = qApp->palette().alternateBase().color();
    QTextBlockFormat bf;
    bf.setBackground(QBrush(bgcolor));
    cursor.mergeBlockFormat(bf);
    cursor.insertText(QString("[%1] %2").arg(timestamp.toString("yyyy-MM-dd HH:mm:ss")).arg(message));
    _ui->journalLog->moveCursor(QTextCursor::End);
    _ui->journalLog->ensureCursorVisible();
}

void MainWindow::setJournal(QMap<QDateTime, QString> _log)
{
    QMap<QDateTime, QString>::const_iterator i = _log.constBegin();
    while (i != _log.constEnd())
    {
        appendToJournalLog(i.key(), i.value());
        ++i;
    }
}

void MainWindow::browseForBackupItems()
{
    FilePickerDialog picker;
    if(picker.exec())
        QMetaObject::invokeMethod(_ui->backupListWidget, "addItemsWithUrls",
                                  QUEUED,
                                  Q_ARG(QList<QUrl>, picker.getSelectedUrls()));
}

void MainWindow::accountMachineUseHostnameButtonClicked()
{
    _ui->accountMachineLineEdit->setText(QHostInfo::localHostName());
    commitSettings();
}

void MainWindow::accountMachineKeyBrowseButtonClicked()
{
    QString key =
        QFileDialog::getOpenFileName(this,
                                     tr("Browse for existing machine key"));
    if(!key.isEmpty())
    {
        _ui->accountMachineKeyLineEdit->setText(key);
        commitSettings();
    }
}

void MainWindow::tarsnapPathBrowseButtonClicked()
{
    QString tarsnapPath =
        QFileDialog::getExistingDirectory(this, tr("Find Tarsnap client"),
                                          _ui->tarsnapPathLineEdit->text());
    if(!tarsnapPath.isEmpty())
    {
        _ui->tarsnapPathLineEdit->setText(tarsnapPath);
        commitSettings();
    }
}

void MainWindow::tarsnapCacheBrowseButton()
{
    QString tarsnapCacheDir =
        QFileDialog::getExistingDirectory(this, tr("Tarsnap cache location"),
                                          _ui->tarsnapCacheLineEdit->text());
    if(!tarsnapCacheDir.isEmpty())
    {
        _ui->tarsnapCacheLineEdit->setText(tarsnapCacheDir);
        commitSettings();
    }
}

void MainWindow::appDataButtonClicked()
{
    QString appDataDir =
        QFileDialog::getExistingDirectory(this,
                                          tr("App data directory location"),
                                          _ui->appDataDirLineEdit->text());
    if(!appDataDir.isEmpty())
    {
        _ui->appDataDirLineEdit->setText(appDataDir);
        commitSettings();
    }
}

void MainWindow::purgeArchivesButtonClicked()
{
    const QString confirmationText = tr("No Tomorrow");
    bool          ok               = false;
    QString       userText         = QInputDialog::getText(
        this, tr("Purge all archives?"),
        tr("This action will <b>delete all (%1) archives</b> stored for this "
           "key."
           "<br /><br />To confirm, type '%2' and press OK."
           "<br /><br /><i>Warning: This action cannot be undone. "
           "All archives will be <b>lost forever</b></i>.")
            .arg(_ui->accountArchivesCountLabel->text(), confirmationText),
        QLineEdit::Normal, "", &ok);
    if(ok && (confirmationText == userText))
    {
        _purgeTimerCount = PURGE_SECONDS_DELAY;
        _purgeCountdownWindow.setText(
            tr("Purging all archives in %1 seconds...").arg(_purgeTimerCount));
        _purgeTimer.start(1000);
        if(QMessageBox::Cancel == _purgeCountdownWindow.exec())
        {
            _purgeTimer.stop();
            updateStatusMessage(tr("Purge cancelled."));
        }
    }
}

void MainWindow::runSetupWizardClicked()
{
    auto confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Reset current app settings, job definitions "
                                 "and run the setup wizard?"));
    if(confirm == QMessageBox::Yes)
        emit runSetupWizard();
}

void MainWindow::expandJournalButtonToggled(bool checked)
{
    if(checked)
        _ui->journalLog->show();
    else
        _ui->journalLog->hide();
}

void MainWindow::downloadsDirBrowseButtonClicked()
{
    QString downDir = QFileDialog::getExistingDirectory(
        this, tr("Browse for downloads directory"),
        QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first());
    if(!downDir.isEmpty())
    {
        _ui->downloadsDirLineEdit->setText(downDir);
        commitSettings();
    }
}

void MainWindow::displayJobDetails(JobPtr job)
{
    hideJobDetails();
    _ui->jobDetailsWidget->setJob(job);
    _ui->jobDetailsWidget->show();
    if(_ui->mainTabWidget->currentWidget() != _ui->jobsTab)
        _ui->mainTabWidget->setCurrentWidget(_ui->jobsTab);
}

void MainWindow::hideJobDetails()
{
    _ui->jobDetailsWidget->hide();
    if(_ui->addJobButton->property("save").toBool())
    {
        _ui->addJobButton->setText(tr("Add job"));
        _ui->addJobButton->setProperty("save", false);
        _ui->addJobButton->setEnabled(true);
    }
}

void MainWindow::addJobClicked()
{
    if(!_ui->addJobButton->isEnabled())
        return;

    if(_ui->addJobButton->property("save").toBool())
    {
        _ui->jobDetailsWidget->saveNew();
        _ui->addJobButton->setText(tr("Add job"));
        _ui->addJobButton->setProperty("save", false);
        _ui->addJobButton->setEnabled(true);
    }
    else
    {
        JobPtr job(new Job());
        displayJobDetails(job);
        _ui->addJobButton->setEnabled(false);
        _ui->addJobButton->setText(tr("Save"));
        _ui->addJobButton->setProperty("save", true);
    }
}

void MainWindow::displayStopTasks(int runningTasks, int queuedTasks)
{
    QMessageBox msgBox;
    msgBox.setText(tr("There are %1 running tasks and %2 queued.")
                       .arg(runningTasks)
                       .arg(queuedTasks));
    msgBox.setInformativeText(tr("What do you want to do?"));
    QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(cancel);
    QPushButton *stopQueued =
        msgBox.addButton(tr("Stop queued"), QMessageBox::ActionRole);
    QPushButton *stopRunning =
        msgBox.addButton(tr("Stop running"), QMessageBox::ActionRole);
    QPushButton *stopAll =
        msgBox.addButton(tr("Stop all"), QMessageBox::ActionRole);
    msgBox.exec();
    if(msgBox.clickedButton() == stopQueued)
    {
        emit stopTasks(false, true);
        updateStatusMessage("Cleared queued tasks.");
    }
    else if(msgBox.clickedButton() == stopRunning)
    {
        emit stopTasks(true, false);
        updateStatusMessage("Stopped running tasks.");
    }
    else if(msgBox.clickedButton() == stopAll)
    {
        emit stopTasks(true, true);
        updateStatusMessage("Stopped running tasks and cleared queued ones.");
    }
}

void MainWindow::tarsnapError(TarsnapError error)
{
    switch(error)
    {
    case TarsnapError::CacheError:
        auto confirm = QMessageBox::critical(this, tr("Tarsnap error"),
                                             tr("The tarsnap cache directory is"
                                                " either missing or is broken."
                                                " Run tarsnap fsck to fix this?\n"),
                                             QMessageBox::Yes | QMessageBox::No);
        if(confirm == QMessageBox::Yes)
            emit repairCache(false);
        break;
    }
}

void MainWindow::updateAccountCredit(qreal credit, QDate date)
{
    QSettings settings;
    settings.setValue("tarsnap/credit", QString::number(credit));
    settings.setValue("tarsnap/credit_date", date);
    _ui->accountCreditLabel->setText(QString::number(credit));
    _ui->accountCreditLabel->setToolTip(date.toString());
}

void MainWindow::updateLastMachineActivity(QStringList activityFields)
{
    QSettings settings;
    settings.setValue("tarsnap/machine_activity", activityFields.join(' '));
    _ui->machineActivity->setText(activityFields.join(' '));
    _ui->machineActivity->resize(_ui->machineActivity->fontMetrics()
                                       .width(_ui->machineActivity->text()) / 2,
                                 _ui->machineActivity->sizeHint().height());
}

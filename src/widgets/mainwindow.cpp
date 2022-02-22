#include "mainwindow.h"

WARNINGS_DISABLE
#include <QAction>
#include <QByteArray>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QEvent>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QSharedPointer>
#include <QStyle>
#include <QStyleOption>
#include <QTabWidget>
#include <QUrl>
#include <QVariant>
#include <Qt>

#include "ui_mainwindow.h"
WARNINGS_ENABLE

#include "ConsoleLog.h"
#include "LogEntry.h"
#include "TBusyLabel.h"
#include "TSettings.h"
#include "TTextView.h"

#include "messages/taskstatus.h"

#include "backuptask.h"
#include "basetask.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "widgets/archivestabwidget.h"
#include "widgets/backuptabwidget.h"
#include "widgets/elidedclickablelabel.h"
#include "widgets/helpwidget.h"
#include "widgets/jobstabwidget.h"
#include "widgets/settingswidget.h"
#include "widgets/stoptasksdialog.h"

#define MAIN_LOGO_RIGHT_MARGIN 5
#define MAIN_LOGO_FUDGE -10

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      _ui(new Ui::MainWindow),
      _minWidth(0),
      _menuBar(nullptr),
      _aboutToQuit(false),
      _backupTaskRunning(false),
      _runningTasks(0),
      _queuedTasks(0),
      _stopTasksDialog(new StopTasksDialog(this)),
      _backupTabWidget(new BackupTabWidget(this)),
      _archivesTabWidget(new ArchivesTabWidget(this)),
      _jobsTabWidget(new JobsTabWidget(this)),
      _settingsWidget(new SettingsWidget(this)),
      _helpWidget(new HelpWidget(this))
{
    connect(&LOG, &ConsoleLog::message, _helpWidget,
            &HelpWidget::appendLogString);

    // Ui initialization
    _ui->setupUi(this);

    displayTab(_ui->backupTab);

    _ui->mainContentSplitter->setCollapsible(0, false);
    _ui->journalLog->hide();

    _ui->backupTabVerticalLayout->addWidget(_backupTabWidget);
    _ui->archivesVerticalLayout->addWidget(_archivesTabWidget);
    _ui->jobsVerticalLayout->addWidget(_jobsTabWidget);
    _ui->settingsTabVerticalLayout->addWidget(_settingsWidget);
    _ui->helpTabLayout->addWidget(_helpWidget);

    connectSettingsWidget();

    // --

    updateUi();
    setupMenuBar();

    // --

    // Ui actions setup
    _ui->settingsTab->addAction(_ui->actionRefreshAccount);
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            &MainWindow::getOverallStats);
    connect(_ui->actionRefreshAccount, &QAction::triggered,
            [this]() { _settingsWidget->getAccountInfo(); });
    addAction(_ui->actionGoBackup);
    addAction(_ui->actionGoArchives);
    addAction(_ui->actionGoJobs);
    addAction(_ui->actionGoSettings);
    addAction(_ui->actionGoHelp);
    connect(_ui->actionGoBackup, &QAction::triggered,
            [this]() { displayTab(_ui->backupTab); });
    connect(_ui->actionGoArchives, &QAction::triggered,
            [this]() { displayTab(_ui->archivesTab); });
    connect(_ui->actionGoJobs, &QAction::triggered,
            [this]() { displayTab(_ui->jobsTab); });
    connect(_ui->actionGoSettings, &QAction::triggered,
            [this]() { displayTab(_ui->settingsTab); });
    connect(_ui->actionGoHelp, &QAction::triggered,
            [this]() { displayTab(_ui->helpTab); });
    addAction(_ui->actionShowJournal);
    connect(_ui->actionShowJournal, &QAction::toggled, _ui->journalLog,
            &QWidget::setVisible);
    addAction(_ui->actionStopTasks);
    connect(_ui->actionStopTasks, &QAction::triggered, this,
            &MainWindow::nonquitStopTasks);
    // --

    // Backup pane
    connect(_backupTabWidget, &BackupTabWidget::morphBackupIntoJob, this,
            &MainWindow::createNewJob);
    connect(_backupTabWidget, &BackupTabWidget::backupNow, this,
            &MainWindow::backupNow);
    connect(_backupTabWidget, &BackupTabWidget::backupTabValidStatus, this,
            &MainWindow::backupTabValidStatus);
    connect(_backupTabWidget, &BackupTabWidget::taskRequested, this,
            &MainWindow::taskRequested);
    connect(_backupTabWidget, &BackupTabWidget::cancelTaskRequested, this,
            &MainWindow::cancelTaskRequested);

    // Handle the Backup-related actions
    connect(_ui->actionBrowseItems, &QAction::triggered, this,
            &MainWindow::browseForBackupItems);
    connect(_ui->actionAddFiles, &QAction::triggered, _backupTabWidget,
            &BackupTabWidget::addFiles);
    connect(_ui->actionAddDirectory, &QAction::triggered, _backupTabWidget,
            &BackupTabWidget::addDirectory);
    connect(_ui->actionClearList, &QAction::triggered, _backupTabWidget,
            &BackupTabWidget::clearList);

    _backupTabWidget->validateBackupTab();

    // Settings pane
    loadSettings();

    connect(_settingsWidget, &SettingsWidget::iecChanged, _backupTabWidget,
            &BackupTabWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::iecChanged, _archivesTabWidget,
            &ArchivesTabWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::iecChanged, _jobsTabWidget,
            &JobsTabWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::iecChanged, _ui->statusBarWidget,
            &StatusBarWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::getArchives, this,
            &MainWindow::getArchives);

    // Archives pane
    connect(this, &MainWindow::archiveList, _archivesTabWidget,
            &ArchivesTabWidget::archiveList);
    connect(this, &MainWindow::addArchive, _archivesTabWidget,
            &ArchivesTabWidget::addArchive);

    connect(_archivesTabWidget, &ArchivesTabWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(_jobsTabWidget, &JobsTabWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_archivesTabWidget, &ArchivesTabWidget::jobClicked, this,
            &MainWindow::jobInspectByRef);
    connect(_archivesTabWidget, &ArchivesTabWidget::displayJobDetails, this,
            &MainWindow::jobInspectByRef);
    connect(_archivesTabWidget, &ArchivesTabWidget::loadArchiveStats, this,
            &MainWindow::loadArchiveStats);
    connect(_archivesTabWidget, &ArchivesTabWidget::loadArchiveContents, this,
            &MainWindow::loadArchiveContents);
    connect(_archivesTabWidget, &ArchivesTabWidget::getArchives, this,
            &MainWindow::getArchives);

    connect(_archivesTabWidget, &ArchivesTabWidget::taskRequested, this,
            &MainWindow::taskRequested);

    // Jobs pane

    // Send menubar actions to the Jobs tab
    connect(_ui->actionAddJob, &QAction::triggered, _jobsTabWidget,
            &JobsTabWidget::addJobClicked);

    // Pass messages through to the JobDetailsWidget
    connect(this, &MainWindow::matchingArchives, _jobsTabWidget,
            &JobsTabWidget::matchingArchives);

    // Pass messages from the JobDetailsWidget
    connect(_jobsTabWidget, &JobsTabWidget::jobAdded, this,
            &MainWindow::jobAdded);
    // The MainWindow::displayJobDetails connection MUST come after the
    // JobListWidget::jobAdded connection.  Otherwise, the JobListWidget won't
    // have the relevant Job in its list when displayJobDetails() tries to
    // select it.
    connect(_jobsTabWidget, &JobsTabWidget::jobAdded, this,
            &MainWindow::displayJobDetails);
    connect(_jobsTabWidget, &JobsTabWidget::displayInspectArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_jobsTabWidget, &JobsTabWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_jobsTabWidget, &JobsTabWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(_jobsTabWidget, &JobsTabWidget::findMatchingArchives, this,
            &MainWindow::findMatchingArchives);
    connect(_jobsTabWidget, &JobsTabWidget::deleteJob, this,
            &MainWindow::deleteJob);

    // Connections to the JobListWidget
    connect(this, &MainWindow::jobList, _jobsTabWidget,
            &JobsTabWidget::jobList);

    // Handle the Job-related actions
    connect(_ui->actionJobBackup, &QAction::triggered, _jobsTabWidget,
            &JobsTabWidget::backupSelectedItems);
    connect(_ui->actionJobDelete, &QAction::triggered, _jobsTabWidget,
            &JobsTabWidget::deleteSelectedItem);
    connect(_ui->actionJobRestore, &QAction::triggered, _jobsTabWidget,
            &JobsTabWidget::restoreSelectedItem);
    connect(_ui->actionJobInspect, &QAction::triggered, _jobsTabWidget,
            &JobsTabWidget::inspectSelectedItem);

    // Other
    connect(_jobsTabWidget, &JobsTabWidget::backupNow, this,
            &MainWindow::backupNow);

    // Connections for _stopTasksDialog
    connect(_stopTasksDialog, &StopTasksDialog::stopTasks, this,
            &MainWindow::stopTasks);
    connect(_stopTasksDialog, &StopTasksDialog::cancelAboutToQuit,
            [this] { _aboutToQuit = false; });
    connect(_stopTasksDialog, &StopTasksDialog::quitOk, this,
            &MainWindow::close);

    // Connections for StatusBarWidget
    connect(_ui->statusBarWidget, &StatusBarWidget::journalToggleRequested,
            _ui->actionShowJournal, &QAction::toggle);
    connect(_ui->statusBarWidget, &StatusBarWidget::stopTasksRequested,
            _ui->actionStopTasks, &QAction::trigger);
}

MainWindow::~MainWindow()
{
    commitSettings();
    delete _stopTasksDialog;
    delete _backupTabWidget;
    delete _archivesTabWidget;
    delete _jobsTabWidget;
    delete _settingsWidget;
    delete _helpWidget;
    delete _ui;
}

void MainWindow::loadSettings()
{
    TSettings settings;

    QByteArray geometry =
        settings.value("app/window_geometry", "").toByteArray();
    if(!geometry.isEmpty())
        restoreGeometry(geometry);
}

void MainWindow::initializeMainWindow()
{
    _settingsWidget->initializeSettingsWidget();

    TSettings settings;

    // Update list of archives (unless we're doing a dry run).
    if(!settings.value("tarsnap/dry_run", false).toBool())
        emit getArchives();
}

void MainWindow::backupTabValidStatus(bool valid)
{
    _ui->actionBackupNow->setEnabled(valid);
    _ui->actionBackupMorphIntoJob->setEnabled(valid);
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    if(_minWidth == 0)
        _minWidth = _ui->mainTabWidget->tabBar()->width();
    // Find out how much room is left to display an icon, including a
    // "fudge factor" to accommodate margins.
    int remaining_width = frameGeometry().width() - _minWidth + MAIN_LOGO_FUDGE;

    // Compare with the width of the png files.
    QPixmap logoPixmap(":/logos/tarsnap-header-h29.png");
    QPixmap iconPixmap(":/logos/tarsnap-icon-h29.png");
    if(remaining_width > logoPixmap.width())
    {
        QIcon icon(":/logos/tarsnap-header-h29.png");
        icon.paint(&p, width() - logoPixmap.width() - MAIN_LOGO_RIGHT_MARGIN, 3,
                   logoPixmap.width(), logoPixmap.height());
    }
    else if(remaining_width > iconPixmap.width())
    {
        QIcon icon(":/logos/tarsnap-icon-h29.png");
        icon.paint(&p, width() - iconPixmap.width() - MAIN_LOGO_RIGHT_MARGIN, 3,
                   iconPixmap.width(), iconPixmap.height());
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(_ui->journalLog->isVisible())
        {
            _ui->actionShowJournal->toggle();
            return;
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Quit if there's no tasks.
    if((_runningTasks == 0) && (_queuedTasks == 0))
    {
        event->accept();
        return;
    }

    // Quit if we're already requested a quit; this occurs when
    // the user selects "Proceed in background".
    if(_aboutToQuit)
    {
        event->accept();
        return;
    }

    // Unless cancelled, we want to quit the next time we have a close().
    _aboutToQuit = true;

    // Ask the user what to do.
    _stopTasksDialog->display(_backupTaskRunning, _runningTasks, _queuedTasks,
                              _aboutToQuit);

    // Don't act on this particular close().
    event->ignore();
}

void MainWindow::nonquitStopTasks()
{
    if((_runningTasks == 0) && (_queuedTasks == 0))
    {
        QMessageBox::information(this, tr("Stop Tasks"),
                                 tr("There are no running or queued tasks."));
    }
    else
    {
        _stopTasksDialog->display(_backupTaskRunning, _runningTasks,
                                  _queuedTasks, _aboutToQuit);
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
        setupMenuBar();
    }
    QWidget::changeEvent(event);
}

void MainWindow::setupMenuBar()
{
    if(_menuBar != nullptr)
    {
        _menuBar->clear();
        delete _menuBar;
        _menuBar = nullptr;
    }

    _menuBar = new QMenuBar(this);
    if(!_menuBar->isNativeMenuBar())
    {
        delete _menuBar;
        _menuBar = nullptr;
        return;
    }

    QAction *actionAbout = new QAction(this);
    actionAbout->setMenuRole(QAction::AboutRole);
    connect(actionAbout, &QAction::triggered, _helpWidget,
            &HelpWidget::aboutMenuClicked);
    QAction *actionSettings = new QAction(this);
    actionSettings->setMenuRole(QAction::PreferencesRole);
    connect(actionSettings, &QAction::triggered, _ui->actionGoSettings,
            &QAction::trigger);
    QMenu *appMenu = _menuBar->addMenu("");
    appMenu->addAction(actionAbout);
    appMenu->addAction(actionSettings);
    QMenu *backupMenu = _menuBar->addMenu(tr("&Backup"));
    backupMenu->addAction(_ui->actionBrowseItems);
    backupMenu->addAction(_ui->actionAddFiles);
    backupMenu->addAction(_ui->actionAddDirectory);
    backupMenu->addAction(_ui->actionClearList);
    backupMenu->addSeparator();
    backupMenu->addAction(_ui->actionBackupNow);
    backupMenu->addAction(_ui->actionBackupMorphIntoJob);
    QMenu *archivesMenu = _menuBar->addMenu(tr("&Archives"));
    archivesMenu->addAction(_ui->actionInspect);
    archivesMenu->addAction(_ui->actionRestore);
    archivesMenu->addAction(_ui->actionDelete);
    archivesMenu->addSeparator();
    archivesMenu->addAction(_ui->actionRefresh);
    archivesMenu->addAction(_ui->actionFilterArchives);
    QMenu *jobsMenu = _menuBar->addMenu(tr("&Jobs"));
    jobsMenu->addAction(_ui->actionJobInspect);
    jobsMenu->addAction(_ui->actionJobRestore);
    jobsMenu->addAction(_ui->actionJobDelete);
    jobsMenu->addSeparator();
    jobsMenu->addAction(_ui->actionJobBackup);
    jobsMenu->addAction(_ui->actionBackupAllJobs);
    jobsMenu->addAction(_ui->actionAddJob);
    jobsMenu->addAction(_ui->actionFilterJobs);
    QMenu *settingsMenu = _menuBar->addMenu(tr("&Settings"));
    settingsMenu->addAction(_ui->actionRefreshAccount);
    settingsMenu->addAction(_ui->actionStopTasks);
    QMenu *windowMenu = _menuBar->addMenu(tr("&Window"));
#ifdef Q_OS_OSX
    QAction *actionMinimize = new QAction(tr("Minimize"), this);
    actionMinimize->setShortcut(QKeySequence("Ctrl+M"));
    connect(actionMinimize, &QAction::triggered, this, &QWidget::showMinimized);
    QAction *actionZoom = new QAction(tr("Zoom"), this);
    connect(actionZoom, &QAction::triggered, this, &QWidget::showMaximized);
    QAction *actionFullScreen = new QAction(tr("Enter Full Screen"), this);
    actionFullScreen->setShortcut(QKeySequence("Ctrl+Meta+F"));
    actionFullScreen->setCheckable(true);
    connect(actionFullScreen, &QAction::triggered, [this](bool checked) {
        if(checked)
        {
            actionFullScreen->setText(tr("Exit Full Screen"));
            this->showFullScreen();
        }
        else
            actionFullScreen->setText(tr("Enter Full Screen"));
        this->showNormal();
    });
    windowMenu->addAction(actionMinimize);
    windowMenu->addAction(actionZoom);
    windowMenu->addAction(actionFullScreen);
    windowMenu->addSeparator();
#endif
    windowMenu->addAction(_ui->actionGoBackup);
    windowMenu->addAction(_ui->actionGoArchives);
    windowMenu->addAction(_ui->actionGoJobs);
    windowMenu->addAction(_ui->actionGoSettings);
    windowMenu->addAction(_ui->actionGoHelp);
    windowMenu->addAction(_ui->actionShowJournal);

    QMenu   *helpMenu             = _menuBar->addMenu(tr("&Help"));
    QAction *actionTarsnapWebsite = new QAction(tr("Tarsnap Website"), this);
    connect(actionTarsnapWebsite, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://www.tarsnap.com"));
    });
    helpMenu->addAction(actionTarsnapWebsite);

    connect(_ui->mainTabWidget, &QTabWidget::currentChanged, this,
            &MainWindow::mainTabChanged);
    mainTabChanged(_ui->mainTabWidget->currentIndex());
}

void MainWindow::mainTabChanged(int index)
{
    Q_UNUSED(index)
    if(_ui->mainTabWidget->currentWidget() == _ui->backupTab)
    {
        _ui->actionBrowseItems->setEnabled(true);
        _backupTabWidget->validateBackupTab();
    }
    else
    {
        _ui->actionBrowseItems->setEnabled(false);
        _ui->actionBackupNow->setEnabled(false);
        _ui->actionBackupMorphIntoJob->setEnabled(false);
    }
    if(_ui->mainTabWidget->currentWidget() == _ui->archivesTab)
    {
        _ui->actionInspect->setEnabled(true);
        _ui->actionRestore->setEnabled(true);
        _ui->actionDelete->setEnabled(true);
        _ui->actionFilterArchives->setEnabled(true);
    }
    else
    {
        _ui->actionInspect->setEnabled(false);
        _ui->actionRestore->setEnabled(false);
        _ui->actionDelete->setEnabled(false);
        _ui->actionFilterArchives->setEnabled(false);
    }
    if(_ui->mainTabWidget->currentWidget() == _ui->jobsTab)
    {
        _ui->actionJobBackup->setEnabled(true);
        _ui->actionJobInspect->setEnabled(true);
        _ui->actionJobRestore->setEnabled(true);
        _ui->actionJobDelete->setEnabled(true);
        _ui->actionFilterJobs->setEnabled(true);
    }
    else
    {
        _ui->actionJobBackup->setEnabled(false);
        _ui->actionJobInspect->setEnabled(false);
        _ui->actionJobRestore->setEnabled(false);
        _ui->actionJobDelete->setEnabled(false);
        _ui->actionFilterJobs->setEnabled(false);
    }
}

void MainWindow::notificationRaise()
{
    raise();
    activateWindow();
    showNormal();
}

void MainWindow::displayInspectArchive(ArchivePtr archive)
{
    displayTab(_ui->archivesTab);
    _archivesTabWidget->displayInspectArchive(archive);
}

void MainWindow::displayJobDetails(JobPtr job)
{
    if(!job)
        return;

    displayTab(_ui->jobsTab);

    _jobsTabWidget->displayJobDetails(job);
}

void MainWindow::updateStatusMessage(const QString &message,
                                     const QString &detail)
{
    _ui->statusBarWidget->updateStatusMessage(message, detail);
}

void MainWindow::commitSettings()
{
    TSettings settings;
    settings.setValue("app/window_geometry", saveGeometry());
    settings.sync();
}

void MainWindow::appendToJournalLog(const LogEntry &log)
{
    _ui->journalLog->appendLog(log);
}

void MainWindow::setJournal(const QVector<LogEntry> &log)
{
    _ui->journalLog->clear();
    for(const LogEntry &entry : log)
        _ui->journalLog->appendLog(entry);
}

void MainWindow::browseForBackupItems()
{
    displayTab(_ui->backupTab);

    _backupTabWidget->browseForBackupItems();
}

void MainWindow::tarsnapError(TarsnapError error)
{
    switch(error)
    {
    case TarsnapError::CacheError:
    {
        QMessageBox::StandardButton confirm =
            QMessageBox::critical(this, tr("Tarsnap error"),
                                  tr("The tarsnap cache directory is"
                                     " either missing or is broken."
                                     " Run tarsnap fsck to fix this?\n"),
                                  QMessageBox::Yes | QMessageBox::No);
        if(confirm == QMessageBox::Yes)
            emit repairCache(false);
        break;
    }
    case TarsnapError::FsckError:
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Cache repair failed. It might be worth trying"
                                 " the 'Repair cache' button in Settings -> "
                                 " Application."));
        break;
    }
    case TarsnapError::NetworkError:
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Tarsnap encountered network errors. Please "
                                 "check your network connection and verify that"
                                 " internet access is available and try "
                                 "again."));
        break;
    }
    }
}

void MainWindow::updateUi()
{
    // Keyboard shortcuts
    _ui->mainTabWidget->setTabToolTip(
        0, _ui->mainTabWidget->tabToolTip(0).arg(
               _ui->actionGoBackup->shortcut().toString(
                   QKeySequence::NativeText)));
    _ui->mainTabWidget->setTabToolTip(
        1, _ui->mainTabWidget->tabToolTip(1).arg(
               _ui->actionGoArchives->shortcut().toString(
                   QKeySequence::NativeText)));
    _ui->mainTabWidget->setTabToolTip(
        2,
        _ui->mainTabWidget->tabToolTip(2).arg(
            _ui->actionGoJobs->shortcut().toString(QKeySequence::NativeText)));
    _ui->mainTabWidget->setTabToolTip(
        3, _ui->mainTabWidget->tabToolTip(3).arg(
               _ui->actionGoSettings->shortcut().toString(
                   QKeySequence::NativeText)));
    _ui->mainTabWidget->setTabToolTip(
        4,
        _ui->mainTabWidget->tabToolTip(4).arg(
            _ui->actionGoHelp->shortcut().toString(QKeySequence::NativeText)));

    _ui->actionBackupNow->setToolTip(_ui->actionBackupNow->toolTip().arg(
        _ui->actionBackupNow->shortcut().toString(QKeySequence::NativeText)));

    _ui->actionShowJournal->setToolTip(_ui->actionShowJournal->toolTip().arg(
        _ui->actionShowJournal->shortcut().toString(QKeySequence::NativeText)));
}

void MainWindow::createNewJob(const QList<QUrl> &urls, const QString &name)
{
    displayTab(_ui->jobsTab);
    _jobsTabWidget->createNewJob(urls, name);
}

void MainWindow::updateNumTasks(bool backupRunning, int runningTasks,
                                int queuedTasks)
{
    if(_stopTasksDialog->isVisible())
        _stopTasksDialog->updateTasks(backupRunning, runningTasks, queuedTasks);

    // Display whether we're active or not.
    bool idle = (runningTasks == 0);
    _ui->statusBarWidget->showBusy(!idle);

    _backupTaskRunning = backupRunning;

    _runningTasks = runningTasks;
    _queuedTasks  = queuedTasks;

    _settingsWidget->updateNumTasks(runningTasks, queuedTasks);
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void MainWindow::overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
                                     quint64 sizeUniqueTotal,
                                     quint64 sizeUniqueCompressed,
                                     quint64 archiveCount)
{
    _settingsWidget->setArchiveCount(archiveCount);
    _ui->statusBarWidget->overallStatsChanged(sizeTotal, sizeCompressed,
                                              sizeUniqueTotal,
                                              sizeUniqueCompressed,
                                              archiveCount);
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void MainWindow::saveKeyId(const QString &key, quint64 id)
{
    _settingsWidget->saveKeyId(key, id);
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void MainWindow::tarsnapVersionResponse(TaskStatus     status,
                                        const QString &versionString)
{
    _settingsWidget->tarsnapVersionResponse(status, versionString);
}

void MainWindow::connectSettingsWidget()
{
    // Get info from SettingsWidget
    connect(_settingsWidget, &SettingsWidget::nukeArchives, this,
            &MainWindow::nukeArchives);
    connect(_settingsWidget, &SettingsWidget::newStatusMessage,
            _ui->statusBarWidget, &StatusBarWidget::updateStatusMessage);
    connect(_settingsWidget, &SettingsWidget::getKeyId, this,
            &MainWindow::getKeyId);
    connect(_settingsWidget, &SettingsWidget::newSimulationStatus,
            _ui->statusBarWidget, &StatusBarWidget::updateSimulationIcon);
    connect(_settingsWidget, &SettingsWidget::clearJournal, this,
            &MainWindow::clearJournal);
    connect(_settingsWidget, &SettingsWidget::runSetupWizard, this,
            &MainWindow::runSetupWizard);
    connect(_settingsWidget, &SettingsWidget::tarsnapVersionRequested, this,
            &MainWindow::tarsnapVersionRequested);
    connect(_settingsWidget, &SettingsWidget::repairCache, this,
            &MainWindow::repairCache);
}

void MainWindow::displayTab(QWidget *widget)
{
    if(_ui->mainTabWidget->currentWidget() != widget)
        _ui->mainTabWidget->setCurrentWidget(widget);
}

void MainWindow::jobInspectByRef(const QString &jobRef)
{
    displayTab(_ui->jobsTab);
    _jobsTabWidget->jobInspectByRef(jobRef);
}

void MainWindow::handle_notification_clicked(enum message_type type,
                                             const QString    &data)
{
    notificationRaise();
    switch(type)
    {
    case(NOTIFICATION_UNSPECIFIED):
    case(NOTIFICATION_NOT_ONLINE):
    case(NOTIFICATION_ARCHIVE_FAILED):
        // Don't navigate to any specific part of the GUI.
        break;
    case(NOTIFICATION_ARCHIVE_CREATING):
        displayTab(_ui->archivesTab);
        break;
    case(NOTIFICATION_ARCHIVE_CREATED):
        displayTab(_ui->archivesTab);
        _archivesTabWidget->displayInspectArchiveByRef(data);
        break;
    }
}

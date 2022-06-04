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

#include "aboutdialog.h"
#include "backuptask.h"
#include "basetask.h"
#include "consolelogdialog.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "widgets/archivestabwidget.h"
#include "widgets/backuptabwidget.h"
#include "widgets/elidedclickablelabel.h"
#include "widgets/helpwidget.h"
#include "widgets/jobstabwidget.h"
#include "widgets/settingswidget.h"
#include "widgets/stoptasksdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      _ui(new Ui::MainWindow),
      _menuBar(nullptr),
      _aboutToQuit(false),
      _backupTaskRunning(false),
      _runningTasks(0),
      _queuedTasks(0),
      _settingsWidget(new SettingsWidget()),
      _aboutWindow(new AboutDialog(this)),
      _consoleWindow(new ConsoleLogDialog(this)),
      _helpWidget(new HelpWidget()),
      _stopTasksDialog(new StopTasksDialog(this))
{
    // Ui initialization
    _ui->setupUi(this);

    displayTab(_ui->backupTabWidget);

    _ui->mainContentSplitter->setCollapsible(0, false);
    _ui->journalLog->hide();

    connectSettingsWidget();

    // --

    updateUi();
    setupMenuBar();

    connect(&LOG, &ConsoleLog::message, _consoleWindow,
            &ConsoleLogDialog::appendLogString);
    connect(_ui->actionShowConsoleLog, &QAction::triggered, _consoleWindow,
            &ConsoleLogDialog::show);

    // --

    // Ui actions setup
    _settingsWidget->addAction(_ui->actionRefreshAccount);
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            &MainWindow::getOverallStats);
    connect(_ui->actionRefreshAccount, &QAction::triggered,
            [this]() { _settingsWidget->getAccountInfo(); });
    addAction(_ui->actionGoBackup);
    addAction(_ui->actionGoArchives);
    addAction(_ui->actionGoJobs);
    connect(_ui->actionGoBackup, &QAction::triggered,
            [this]() { displayTab(_ui->backupTabWidget); });
    connect(_ui->actionGoArchives, &QAction::triggered,
            [this]() { displayTab(_ui->archivesTabWidget); });
    connect(_ui->actionGoJobs, &QAction::triggered,
            [this]() { displayTab(_ui->jobsTabWidget); });
    addAction(_ui->actionShowJournal);
    connect(_ui->actionShowJournal, &QAction::toggled, _ui->journalLog,
            &QWidget::setVisible);
    addAction(_ui->actionStopTasks);
    connect(_ui->actionStopTasks, &QAction::triggered, this,
            &MainWindow::nonquitStopTasks);
    // --

    // Backup pane
    connect(_ui->backupTabWidget, &BackupTabWidget::morphBackupIntoJob, this,
            &MainWindow::createNewJob);
    connect(_ui->backupTabWidget, &BackupTabWidget::backupNow, this,
            &MainWindow::backupNow);
    connect(_ui->backupTabWidget, &BackupTabWidget::backupTabValidStatus, this,
            &MainWindow::backupTabValidStatus);
    connect(_ui->backupTabWidget, &BackupTabWidget::taskRequested, this,
            &MainWindow::taskRequested);
    connect(_ui->backupTabWidget, &BackupTabWidget::cancelTaskRequested, this,
            &MainWindow::cancelTaskRequested);

    // Handle the Backup-related actions
    connect(_ui->actionBrowseItems, &QAction::triggered, this,
            &MainWindow::browseForBackupItems);
    connect(_ui->actionAddFiles, &QAction::triggered, _ui->backupTabWidget,
            &BackupTabWidget::addFiles);
    connect(_ui->actionAddDirectory, &QAction::triggered, _ui->backupTabWidget,
            &BackupTabWidget::addDirectory);
    connect(_ui->actionClearList, &QAction::triggered, _ui->backupTabWidget,
            &BackupTabWidget::clearList);

    _ui->backupTabWidget->validateBackupTab();

    // Settings pane
    loadSettings();

    connect(_settingsWidget, &SettingsWidget::iecChanged, _ui->backupTabWidget,
            &BackupTabWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::iecChanged,
            _ui->archivesTabWidget, &ArchivesTabWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::iecChanged, _ui->jobsTabWidget,
            &JobsTabWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::iecChanged, _ui->statusBarWidget,
            &StatusBarWidget::updateIEC);
    connect(_settingsWidget, &SettingsWidget::getArchives, this,
            &MainWindow::getArchives);

    // Archives pane
    connect(this, &MainWindow::archiveList, _ui->archivesTabWidget,
            &ArchivesTabWidget::archiveList);
    connect(this, &MainWindow::addArchive, _ui->archivesTabWidget,
            &ArchivesTabWidget::addArchive);

    connect(_ui->archivesTabWidget, &ArchivesTabWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui->jobsTabWidget, &JobsTabWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui->archivesTabWidget, &ArchivesTabWidget::jobClicked, this,
            &MainWindow::jobInspectByRef);
    connect(_ui->archivesTabWidget, &ArchivesTabWidget::displayJobDetails, this,
            &MainWindow::jobInspectByRef);
    connect(_ui->archivesTabWidget, &ArchivesTabWidget::loadArchiveStats, this,
            &MainWindow::loadArchiveStats);
    connect(_ui->archivesTabWidget, &ArchivesTabWidget::loadArchiveContents,
            this, &MainWindow::loadArchiveContents);
    connect(_ui->archivesTabWidget, &ArchivesTabWidget::getArchives, this,
            &MainWindow::getArchives);

    connect(_ui->archivesTabWidget, &ArchivesTabWidget::taskRequested, this,
            &MainWindow::taskRequested);

    // Jobs pane

    // Send menubar actions to the Jobs tab
    connect(_ui->actionAddJob, &QAction::triggered, _ui->jobsTabWidget,
            &JobsTabWidget::addJobClicked);

    // Pass messages through to the JobDetailsWidget
    connect(this, &MainWindow::matchingArchives, _ui->jobsTabWidget,
            &JobsTabWidget::matchingArchives);

    // Pass messages from the JobDetailsWidget
    connect(_ui->jobsTabWidget, &JobsTabWidget::jobAdded, this,
            &MainWindow::jobAdded);
    // The MainWindow::displayJobDetails connection MUST come after the
    // JobListWidget::jobAdded connection.  Otherwise, the JobListWidget won't
    // have the relevant Job in its list when displayJobDetails() tries to
    // select it.
    connect(_ui->jobsTabWidget, &JobsTabWidget::jobAdded, this,
            &MainWindow::displayJobDetails);
    connect(_ui->jobsTabWidget, &JobsTabWidget::displayInspectArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_ui->jobsTabWidget, &JobsTabWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui->jobsTabWidget, &JobsTabWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui->jobsTabWidget, &JobsTabWidget::findMatchingArchives, this,
            &MainWindow::findMatchingArchives);
    connect(_ui->jobsTabWidget, &JobsTabWidget::deleteJob, this,
            &MainWindow::deleteJob);

    // Connections to the JobListWidget
    connect(this, &MainWindow::jobList, _ui->jobsTabWidget,
            &JobsTabWidget::jobList);

    // Handle the Job-related actions
    connect(_ui->actionJobBackup, &QAction::triggered, _ui->jobsTabWidget,
            &JobsTabWidget::backupSelectedItems);
    connect(_ui->actionJobDelete, &QAction::triggered, _ui->jobsTabWidget,
            &JobsTabWidget::deleteSelectedItem);
    connect(_ui->actionJobRestore, &QAction::triggered, _ui->jobsTabWidget,
            &JobsTabWidget::restoreSelectedItem);
    connect(_ui->actionJobInspect, &QAction::triggered, _ui->jobsTabWidget,
            &JobsTabWidget::inspectSelectedItem);

    // Other
    connect(_ui->jobsTabWidget, &JobsTabWidget::backupNow, this,
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
    }
    QWidget::changeEvent(event);
}

void MainWindow::setupMenuBar()
{
    connect(_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(_ui->actionAbout, &QAction::triggered, _aboutWindow,
            &AboutDialog::show);
    connect(_ui->actionSettings, &QAction::triggered, _settingsWidget,
            &SettingsWidget::show);
    connect(_ui->actionMinimize, &QAction::triggered, this,
            &QWidget::showMinimized);
    connect(_ui->actionZoom, &QAction::triggered, this,
            &QWidget::showMaximized);
    connect(_ui->actionFullScreen, &QAction::triggered, [this](bool checked) {
        if(checked)
        {
            _ui->actionFullScreen->setText(tr("Exit Full Screen"));
            this->showFullScreen();
        }
        else
            _ui->actionFullScreen->setText(tr("Enter Full Screen"));
        this->showNormal();
    });
#ifdef Q_OS_OSX
    // Remove the file menu, because macOS will add an "app menu".
    _ui->menu_File->menuAction()->setVisible(false);

    // Leave the three Windows actions enabled and visible.
#else
    // Set default shortcut for showing the help.
    // This isn't enabled on macOS because on that platform, this shortcut
    // automatically opens the "Help" menu.  This does seem slightly odd
    // to me, but we shouldn't deviate from the typical platform behaviour
    // unless we have a really good reason, and we don't.
    _ui->actionHelp->setShortcut(QKeySequence::HelpContents);

    // Disable the three Windows actions.
    for(QAction *action :
        {_ui->actionFullScreen, _ui->actionMinimize, _ui->actionZoom})
    {
        action->setEnabled(false);
        action->setVisible(false);
    }
#endif
    connect(_ui->actionTarsnapWebsite, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://www.tarsnap.com"));
    });
    connect(_ui->actionHelp, &QAction::triggered, _helpWidget,
            &HelpWidget::show);

    connect(_ui->mainTabWidget, &QTabWidget::currentChanged, this,
            &MainWindow::mainTabChanged);
    mainTabChanged(_ui->mainTabWidget->currentIndex());
}

void MainWindow::mainTabChanged(int index)
{
    Q_UNUSED(index)
    if(_ui->mainTabWidget->currentWidget() == _ui->backupTabWidget)
    {
        _ui->actionBrowseItems->setEnabled(true);
        _ui->backupTabWidget->validateBackupTab();
    }
    else
    {
        _ui->actionBrowseItems->setEnabled(false);
        _ui->actionBackupNow->setEnabled(false);
        _ui->actionBackupMorphIntoJob->setEnabled(false);
    }
    if(_ui->mainTabWidget->currentWidget() == _ui->archivesTabWidget)
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
    if(_ui->mainTabWidget->currentWidget() == _ui->jobsTabWidget)
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

void MainWindow::displayInspectArchive(const ArchivePtr &archive)
{
    displayTab(_ui->archivesTabWidget);
    _ui->archivesTabWidget->displayInspectArchive(archive);
}

void MainWindow::displayJobDetails(const JobPtr &job)
{
    if(!job)
        return;

    displayTab(_ui->jobsTabWidget);

    _ui->jobsTabWidget->displayJobDetails(job);
}

void MainWindow::updateStatusMessage(const QString &message)
{
    _ui->statusBarWidget->updateStatusMessage(message);
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
    displayTab(_ui->backupTabWidget);

    _ui->backupTabWidget->browseForBackupItems();
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

    _ui->actionBackupNow->setToolTip(_ui->actionBackupNow->toolTip().arg(
        _ui->actionBackupNow->shortcut().toString(QKeySequence::NativeText)));

    _ui->actionShowJournal->setToolTip(_ui->actionShowJournal->toolTip().arg(
        _ui->actionShowJournal->shortcut().toString(QKeySequence::NativeText)));
}

void MainWindow::createNewJob(const QList<QUrl> &urls, const QString &name)
{
    displayTab(_ui->jobsTabWidget);
    _ui->jobsTabWidget->createNewJob(urls, name);
}

void MainWindow::updateNumTasks(bool backupRunning, int numRunning,
                                int numQueued)
{
    if(_stopTasksDialog->isVisible())
        _stopTasksDialog->updateTasks(backupRunning, numRunning, numQueued);

    // Display whether we're active or not.
    bool idle = (numRunning == 0);
    _ui->statusBarWidget->showBusy(!idle);

    _backupTaskRunning = backupRunning;

    _runningTasks = numRunning;
    _queuedTasks  = numQueued;

    _settingsWidget->updateNumTasks(numRunning, numQueued);
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
    displayTab(_ui->jobsTabWidget);
    _ui->jobsTabWidget->jobInspectByRef(jobRef);
}

void MainWindow::handle_notification_clicked(enum message_type type,
                                             const QString    &data)
{
    notificationRaise();
    switch(type)
    {
    case(NOTIFICATION_NONE):
        qFatal("Uninitialized notification type");
    case(NOTIFICATION_UNSPECIFIED):
    case(NOTIFICATION_NOT_ONLINE):
    case(NOTIFICATION_ARCHIVE_FAILED):
        // Don't navigate to any specific part of the GUI.
        break;
    case(NOTIFICATION_ARCHIVE_CREATING):
        displayTab(_ui->archivesTabWidget);
        break;
    case(NOTIFICATION_ARCHIVE_CREATED):
        displayTab(_ui->archivesTabWidget);
        _ui->archivesTabWidget->displayInspectArchiveByRef(data);
        break;
    }
}

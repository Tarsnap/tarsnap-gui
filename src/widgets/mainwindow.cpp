#include "mainwindow.h"

WARNINGS_DISABLE
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QHostInfo>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QSharedPointer>
#include <QShortcut>

#include "ui_mainwindow.h"
WARNINGS_ENABLE

#include "backuplistwidgetitem.h"
#include "scheduling.h"
#include "translator.h"
#include "utils.h"

#include <ConsoleLog.h>
#include <TSettings.h>

#define MAIN_LOGO_RIGHT_MARGIN 5
#define MAIN_LOGO_FUDGE 3

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::MainWindow),
      _minWidth(0),
      _menuBar(nullptr),
      _aboutToQuit(false),
      _stopTasksDialog(this),
      _backupTabWidget(this),
      _archivesTabWidget(this),
      _jobsTabWidget(this),
      _settingsWidget(this),
      _helpWidget(this)
{
    connect(&LOG, &ConsoleLog::message, this, &MainWindow::appendToConsoleLog);

    // Ui initialization
    _ui->setupUi(this);

    displayTab(_ui->backupTab);

    _ui->mainContentSplitter->setCollapsible(0, false);
    _ui->journalLog->hide();

    _ui->backupTabVerticalLayout->addWidget(&_backupTabWidget);
    _ui->archivesVerticalLayout->addWidget(&_archivesTabWidget);
    _ui->jobsVerticalLayout->addWidget(&_jobsTabWidget);
    _ui->settingsTabVerticalLayout->addWidget(&_settingsWidget);
    _ui->helpTabLayout->addWidget(&_helpWidget);

    connectSettingsWidget();

    // --

    updateUi();

    // --

    // Ui actions setup
    _ui->settingsTab->addAction(_ui->actionRefreshAccount);
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            &MainWindow::getOverallStats);
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            [&]() { _settingsWidget.getAccountInfo(); });
    addAction(_ui->actionGoBackup);
    addAction(_ui->actionGoArchives);
    addAction(_ui->actionGoJobs);
    addAction(_ui->actionGoSettings);
    addAction(_ui->actionGoHelp);
    connect(_ui->actionGoBackup, &QAction::triggered,
            [&]() { displayTab(_ui->backupTab); });
    connect(_ui->actionGoArchives, &QAction::triggered,
            [&]() { displayTab(_ui->archivesTab); });
    connect(_ui->actionGoJobs, &QAction::triggered,
            [&]() { displayTab(_ui->jobsTab); });
    connect(_ui->actionGoSettings, &QAction::triggered,
            [&]() { displayTab(_ui->settingsTab); });
    connect(_ui->actionGoHelp, &QAction::triggered,
            [&]() { displayTab(_ui->helpTab); });
    addAction(_ui->actionShowJournal);
    _ui->expandJournalButton->setDefaultAction(_ui->actionShowJournal);
    connect(_ui->actionShowJournal, &QAction::toggled, _ui->journalLog,
            &QWidget::setVisible);
    connect(_ui->statusBarLabel, &ElidedClickableLabel::clicked,
            _ui->actionShowJournal, &QAction::toggle);
    addAction(_ui->actionStopTasks);
    connect(_ui->actionStopTasks, &QAction::triggered, this,
            &MainWindow::getTaskInfo);
    connect(_ui->busyWidget, &BusyWidget::clicked, _ui->actionStopTasks,
            &QAction::trigger);
    // --

    // Backup pane
    connect(&_backupTabWidget, &BackupTabWidget::morphBackupIntoJob, this,
            &MainWindow::createNewJob);
    connect(&_backupTabWidget, &BackupTabWidget::backupNow, this,
            &MainWindow::backupNow);
    connect(&_backupTabWidget, &BackupTabWidget::backupTabValidStatus, this,
            &MainWindow::backupTabValidStatus);

    // Handle the Backup-related actions
    connect(_ui->actionBrowseItems, &QAction::triggered, this,
            &MainWindow::browseForBackupItems);
    connect(_ui->actionAddFiles, &QAction::triggered, &_backupTabWidget,
            &BackupTabWidget::addFiles);
    connect(_ui->actionAddDirectory, &QAction::triggered, &_backupTabWidget,
            &BackupTabWidget::addDirectory);
    connect(_ui->actionClearList, &QAction::triggered, &_backupTabWidget,
            &BackupTabWidget::clearList);

    _backupTabWidget.validateBackupTab();

    // Settings pane
    loadSettings();

    // Archives pane
    connect(this, &MainWindow::archiveList, &_archivesTabWidget,
            &ArchivesTabWidget::archiveList);
    connect(this, &MainWindow::addArchive, &_archivesTabWidget,
            &ArchivesTabWidget::addArchive);

    connect(&_archivesTabWidget, &ArchivesTabWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(&_jobsTabWidget, &JobsTabWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(&_archivesTabWidget, &ArchivesTabWidget::jobClicked, this,
            &MainWindow::jobInspectByRef);
    connect(&_archivesTabWidget, &ArchivesTabWidget::displayJobDetails, this,
            &MainWindow::jobInspectByRef);
    connect(&_archivesTabWidget, &ArchivesTabWidget::loadArchiveStats, this,
            &MainWindow::loadArchiveStats);
    connect(&_archivesTabWidget, &ArchivesTabWidget::loadArchiveContents, this,
            &MainWindow::loadArchiveContents);

    connect(_ui->actionRefresh, &QAction::triggered, this,
            &MainWindow::getArchives);

    // Jobs pane

    // Send menubar actions to the Jobs tab
    connect(_ui->actionAddJob, &QAction::triggered, &_jobsTabWidget,
            &JobsTabWidget::addJobClicked);

    // Pass messages through to the JobDetailsWidget
    connect(this, &MainWindow::matchingArchives, &_jobsTabWidget,
            &JobsTabWidget::matchingArchives);

    // Pass messages from the JobDetailsWidget
    connect(&_jobsTabWidget, &JobsTabWidget::jobAdded, this,
            &MainWindow::jobAdded);
    // The MainWindow::displayJobDetails connection MUST come after the
    // JobListWidget::jobAdded connection.  Otherwise, the JobListWidget won't
    // have the relevant Job in its list when displayJobDetails() tries to
    // select it.
    connect(&_jobsTabWidget, &JobsTabWidget::jobAdded, this,
            &MainWindow::displayJobDetails);
    connect(&_jobsTabWidget, &JobsTabWidget::displayInspectArchive, this,
            &MainWindow::displayInspectArchive);
    connect(&_jobsTabWidget, &JobsTabWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(&_jobsTabWidget, &JobsTabWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(&_jobsTabWidget, &JobsTabWidget::findMatchingArchives, this,
            &MainWindow::findMatchingArchives);
    connect(&_jobsTabWidget, &JobsTabWidget::deleteJob, this,
            &MainWindow::deleteJob);

    // Connections to the JobListWidget
    connect(this, &MainWindow::jobList, &_jobsTabWidget,
            &JobsTabWidget::jobList);

    // Handle the Job-related actions
    connect(_ui->actionJobBackup, &QAction::triggered, &_jobsTabWidget,
            &JobsTabWidget::backupSelectedItems);
    connect(_ui->actionJobDelete, &QAction::triggered, &_jobsTabWidget,
            &JobsTabWidget::deleteSelectedItem);
    connect(_ui->actionJobRestore, &QAction::triggered, &_jobsTabWidget,
            &JobsTabWidget::restoreSelectedItem);
    connect(_ui->actionJobInspect, &QAction::triggered, &_jobsTabWidget,
            &JobsTabWidget::inspectSelectedItem);

    // Other
    connect(&_jobsTabWidget, &JobsTabWidget::backupNow, this,
            &MainWindow::backupNow);

    _consoleLog = _helpWidget.getConsoleLog();
}

MainWindow::~MainWindow()
{
    commitSettings();
    delete _ui;
}

void MainWindow::loadSettings()
{
    TSettings settings;

    _ui->simulationIcon->setVisible(
        settings.value("tarsnap/dry_run", DEFAULT_DRY_RUN).toBool());

    QByteArray geometry =
        settings.value("app/window_geometry", "").toByteArray();
    if(!geometry.isEmpty())
        restoreGeometry(geometry);
}

void MainWindow::initializeMainWindow()
{
    _settingsWidget.initializeSettingsWidget();

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
        _minWidth = minimumWidth();
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
    if(_aboutToQuit)
    {
        event->accept();
    }
    else
    {
        _aboutToQuit = true;
        // Find out if there's any running or queued tasks, so that we can
        // ask the user what to do (if necessary).
        emit getTaskInfo();
        event->ignore();
    }
}

void MainWindow::closeWithTaskInfo(bool backupTaskRunning, int runningTasks,
                                   int queuedTasks)
{
    if(!runningTasks && !queuedTasks)
    {
        if(_aboutToQuit)
        {
            qApp->quit();
            return;
        }
        else
        {
            QMessageBox::information(
                this, tr("Stop Tasks"),
                tr("There are no running or queued tasks."));
            return;
        }
    }

    displayStopTasksDialog(backupTaskRunning, runningTasks, queuedTasks);
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();

        // Clear previous-language status message
        updateStatusMessage("", "");
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
    connect(actionAbout, &QAction::triggered, &_helpWidget,
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
    connect(actionFullScreen, &QAction::triggered, [=](bool checked) {
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

    QMenu *  helpMenu             = _menuBar->addMenu(tr("&Help"));
    QAction *actionTarsnapWebsite = new QAction(tr("Tarsnap Website"), this);
    connect(actionTarsnapWebsite, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://www.tarsnap.com"));
    });
    helpMenu->addAction(actionTarsnapWebsite);

    connect(_ui->mainTabWidget, &QTabWidget::currentChanged, this,
            &MainWindow::mainTabChanged);
    mainTabChanged(_ui->mainTabWidget->currentIndex());
}

void MainWindow::updateLoadingAnimation(bool idle)
{
    if(idle)
        _ui->busyWidget->stop();
    else
        _ui->busyWidget->animate();
}

void MainWindow::mainTabChanged(int index)
{
    Q_UNUSED(index)
    if(_ui->mainTabWidget->currentWidget() == _ui->backupTab)
    {
        _ui->actionBrowseItems->setEnabled(true);
        _backupTabWidget.validateBackupTab();
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
    _archivesTabWidget.displayInspectArchive(archive);
}

void MainWindow::displayJobDetails(JobPtr job)
{
    if(!job)
        return;

    displayTab(_ui->jobsTab);

    _jobsTabWidget.displayJobDetails(job);
}

void MainWindow::updateStatusMessage(QString message, QString detail)
{
    _ui->statusBarLabel->setText(message);
    if(!detail.isEmpty())
        _ui->statusBarLabel->setToolTip(detail);
}

void MainWindow::commitSettings()
{
    TSettings settings;
    settings.setValue("app/window_geometry", saveGeometry());
    settings.sync();
}

void MainWindow::appendToJournalLog(LogEntry log)
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
    cursor.insertText(
        QString("[%1] %2")
            .arg(log.timestamp.toString(Qt::DefaultLocaleShortDate))
            .arg(log.message));
    _ui->journalLog->moveCursor(QTextCursor::End);
    _ui->journalLog->ensureCursorVisible();
}

void MainWindow::appendToConsoleLog(const QString &log)
{
    QTextCursor cursor(_consoleLog->document());
    if(!_consoleLog->document()->isEmpty())
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertBlock();
        cursor.movePosition(QTextCursor::NextBlock);
    }
    QTextBlockFormat bf;
    if(cursor.blockFormat().background().color()
       == qApp->palette().base().color())
        bf.setBackground(QBrush(qApp->palette().alternateBase().color()));
    else
        bf.setBackground(QBrush(qApp->palette().base().color()));
    cursor.mergeBlockFormat(bf);
    cursor.insertText(QString("[%1] %2")
                          .arg(QDateTime::currentDateTime().toString(
                              Qt::DefaultLocaleShortDate))
                          .arg(log));
    _consoleLog->moveCursor(QTextCursor::End);
    _consoleLog->ensureCursorVisible();
}

void MainWindow::setJournal(QVector<LogEntry> log)
{
    _ui->journalLog->clear();
    for(const LogEntry &entry : log)
        appendToJournalLog(entry);
}

void MainWindow::browseForBackupItems()
{
    displayTab(_ui->backupTab);

    _backupTabWidget.browseForBackupItems();
}

void MainWindow::displayStopTasksDialog(bool backupTaskRunning,
                                        int runningTasks, int queuedTasks)
{
    _stopTasksDialog.setText(tr("There are %1 running tasks and %2 queued.")
                                 .arg(runningTasks)
                                 .arg(queuedTasks));
    _stopTasksDialog.setInformativeText(tr("What do you want to do?"));

    QPushButton actionButton(&_stopTasksDialog);
    actionButton.setText(tr("Choose action"));
    QMenu    actionMenu(&actionButton);
    QAction *interruptBackup = nullptr;
    if(backupTaskRunning)
    {
        if(_aboutToQuit)
            interruptBackup =
                actionMenu.addAction(tr("Interrupt backup and clear queue"));
        else
            interruptBackup = actionMenu.addAction(tr("Interrupt backup"));
        interruptBackup->setCheckable(true);
    }
    QAction *stopRunning = nullptr;
    if(runningTasks && !_aboutToQuit)
    {
        stopRunning = actionMenu.addAction(tr("Stop running"));
        stopRunning->setCheckable(true);
    }
    QAction *stopQueued = nullptr;
    if(queuedTasks && !_aboutToQuit)
    {
        stopQueued = actionMenu.addAction(tr("Cancel queued"));
        stopQueued->setCheckable(true);
    }
    QAction *stopAll = nullptr;
    if(runningTasks || queuedTasks)
    {
        stopAll = actionMenu.addAction(tr("Stop all"));
        stopAll->setCheckable(true);
    }
    QAction *proceedBackground = nullptr;
    if((runningTasks || queuedTasks) && _aboutToQuit)
    {
        proceedBackground = actionMenu.addAction(tr("Proceed in background"));
        proceedBackground->setCheckable(true);
    }
    QPushButton *cancel = _stopTasksDialog.addButton(QMessageBox::Cancel);
    _stopTasksDialog.setDefaultButton(cancel);
    connect(&actionMenu, &QMenu::triggered, &_stopTasksDialog, &QDialog::accept,
            Qt::QueuedConnection);
    actionButton.setMenu(&actionMenu);
    _stopTasksDialog.addButton(&actionButton, QMessageBox::ActionRole);
    _stopTasksDialog.exec();

    if((_stopTasksDialog.clickedButton() == cancel) && _aboutToQuit)
        _aboutToQuit = false;

    if(_aboutToQuit)
    {
        qApp->setQuitLockEnabled(true);
        close();
    }

    if(interruptBackup && interruptBackup->isChecked())
        emit stopTasks(true, false, _aboutToQuit);
    else if(stopQueued && stopQueued->isChecked())
        emit stopTasks(false, false, true);
    else if(stopRunning && stopRunning->isChecked())
        emit stopTasks(false, true, false);
    else if(stopAll && stopAll->isChecked())
        emit stopTasks(false, true, true);
    else if(proceedBackground && proceedBackground->isChecked())
    {
        // Do nothing; it will happen due to code elsewhere
    }
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
    _ui->busyWidget->setToolTip(_ui->busyWidget->toolTip().arg(
        _ui->actionStopTasks->shortcut().toString(QKeySequence::NativeText)));

    // --

    setupMenuBar();
}

void MainWindow::createNewJob(QList<QUrl> urls, QString name)
{
    displayTab(_ui->jobsTab);
    _jobsTabWidget.createNewJob(urls, name);
}

void MainWindow::updateSimulationIcon(int state)
{
    if(state == Qt::Unchecked)
    {
        emit getArchives();
        _ui->simulationIcon->hide();
    }
    else
    {
        _ui->simulationIcon->show();
    }
}

void MainWindow::updateNumTasks(int runningTasks, int queuedTasks)
{
    _runningTasks = runningTasks;
    _queuedTasks  = queuedTasks;

    _settingsWidget.updateNumTasks(runningTasks, queuedTasks);
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void MainWindow::overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
                                     quint64 sizeUniqueTotal,
                                     quint64 sizeUniqueCompressed,
                                     quint64 archiveCount)
{
    _settingsWidget.overallStatsChanged(sizeTotal, sizeCompressed,
                                        sizeUniqueTotal, sizeUniqueCompressed,
                                        archiveCount);
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void MainWindow::saveKeyId(QString key, quint64 id)
{
    _settingsWidget.saveKeyId(key, id);
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void MainWindow::tarsnapVersionResponse(QString versionString)
{
    _settingsWidget.tarsnapVersionResponse(versionString);
}

void MainWindow::connectSettingsWidget()
{
    // Get info from SettingsWidget
    connect(&_settingsWidget, &SettingsWidget::nukeArchives, this,
            &MainWindow::nukeArchives);
    connect(&_settingsWidget, &SettingsWidget::newStatusMessage, this,
            &MainWindow::updateStatusMessage);
    connect(&_settingsWidget, &SettingsWidget::getKeyId, this,
            &MainWindow::getKeyId);
    connect(&_settingsWidget, &SettingsWidget::newSimulationStatus, this,
            &MainWindow::updateSimulationIcon);
    connect(&_settingsWidget, &SettingsWidget::clearJournal, this,
            &MainWindow::clearJournal);
    connect(&_settingsWidget, &SettingsWidget::runSetupWizard, this,
            &MainWindow::runSetupWizard);
    connect(&_settingsWidget, &SettingsWidget::tarsnapVersionRequested, this,
            &MainWindow::tarsnapVersionRequested);
    connect(&_settingsWidget, &SettingsWidget::repairCache, this,
            &MainWindow::repairCache);
}

void MainWindow::displayTab(QWidget *widget)
{
    if(_ui->mainTabWidget->currentWidget() != widget)
        _ui->mainTabWidget->setCurrentWidget(widget);
}

void MainWindow::jobInspectByRef(QString jobRef)
{
    displayTab(_ui->jobsTab);
    _jobsTabWidget.jobInspectByRef(jobRef);
}

void MainWindow::handle_notification_clicked(enum message_type type,
                                             QString           data)
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
        _archivesTabWidget.displayInspectArchiveByRef(data);
        break;
    }
}

#include "mainwindow.h"
#include "backuplistwidgetitem.h"
#include "debug.h"
#include "scheduling.h"
#include "translator.h"
#include "utils.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QHostInfo>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>
#include <QSharedPointer>
#include <QShortcut>

#include <TSettings.h>

#define MAIN_LOGO_RIGHT_MARGIN 5
#define MAIN_LOGO_FUDGE 3

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      _minWidth(0),
      _menuBar(nullptr),
      _aboutToQuit(false),
      _stopTasksDialog(this),
      _filePickerDialog(this),
      _archivesTabWidget(this),
      _settingsWidget(this),
      _helpWidget(this)
{
    connect(&ConsoleLog::instance(), &ConsoleLog::message, this,
            &MainWindow::appendToConsoleLog);

    // Ui initialization
    _ui.setupUi(this);
    _ui.backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui.jobListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    displayTab(_ui.backupTab);
    validateBackupTab();
    _ui.mainContentSplitter->setCollapsible(0, false);
    _ui.journalLog->hide();
    _ui.jobDetailsWidget->hide();
    _ui.jobsFilterFrame->hide();

    _ui.archivesVerticalLayout->insertWidget(0, &_archivesTabWidget);
    _ui.settingsTabVerticalLayout->insertWidget(0, &_settingsWidget);
    _ui.helpTabLayout->insertWidget(0, &_helpWidget);

    connectSettingsWidget();

    // --

    updateUi();

    // --

    // Ui actions setup
    _ui.settingsTab->addAction(_ui.actionRefreshAccount);
    connect(_ui.actionRefreshAccount, &QAction::triggered, this,
            &MainWindow::getOverallStats);
    connect(_ui.actionRefreshAccount, &QAction::triggered, this,
            [&]() { _settingsWidget.getAccountInfo(); });
    addAction(_ui.actionGoBackup);
    addAction(_ui.actionGoArchives);
    addAction(_ui.actionGoJobs);
    addAction(_ui.actionGoSettings);
    addAction(_ui.actionGoHelp);
    connect(_ui.actionGoBackup, &QAction::triggered,
            [&]() { displayTab(_ui.backupTab); });
    connect(_ui.actionGoArchives, &QAction::triggered,
            [&]() { displayTab(_ui.archivesTab); });
    connect(_ui.actionGoJobs, &QAction::triggered,
            [&]() { displayTab(_ui.jobsTab); });
    connect(_ui.actionGoSettings, &QAction::triggered,
            [&]() { displayTab(_ui.settingsTab); });
    connect(_ui.actionGoHelp, &QAction::triggered,
            [&]() { displayTab(_ui.helpTab); });
    addAction(_ui.actionShowJournal);
    _ui.expandJournalButton->setDefaultAction(_ui.actionShowJournal);
    connect(_ui.actionShowJournal, &QAction::toggled, _ui.journalLog,
            &QWidget::setVisible);
    connect(_ui.statusBarLabel, &ElidedLabel::clicked, _ui.actionShowJournal,
            &QAction::toggle);
    addAction(_ui.actionStopTasks);
    connect(_ui.actionStopTasks, &QAction::triggered, this,
            &MainWindow::getTaskInfo);
    connect(_ui.busyWidget, &BusyWidget::clicked, _ui.actionStopTasks,
            &QAction::trigger);
    addAction(_ui.actionShowArchivesTabHeader);
    addAction(_ui.actionShowJobsTabHeader);
    // --

    // Backup pane
    _ui.backupButton->setDefaultAction(_ui.actionBackupNow);
    _ui.backupButton->addAction(_ui.actionCreateJob);
    connect(_ui.actionBackupNow, &QAction::triggered, this,
            &MainWindow::backupButtonClicked);
    connect(_ui.actionCreateJob, &QAction::triggered, this,
            &MainWindow::createJobClicked);
    connect(_ui.backupListWidget, &BackupListWidget::itemTotals, this,
            &MainWindow::updateBackupItemTotals);
    _ui.backupListWidget->addAction(_ui.actionBrowseItems);
    _ui.backupListWidget->addAction(_ui.actionAddFiles);
    _ui.backupListWidget->addAction(_ui.actionAddDirectory);
    _ui.backupListWidget->addAction(_ui.actionClearList);
    connect(_ui.actionClearList, &QAction::triggered, _ui.backupListWidget,
            &BackupListWidget::clear);
    connect(_ui.actionBrowseItems, &QAction::triggered, this,
            &MainWindow::browseForBackupItems);
    connect(_ui.backupListInfoLabel, &ElidedLabel::clicked,
            _ui.actionBrowseItems, &QAction::trigger);
    connect(_ui.appendTimestampCheckBox, &QCheckBox::toggled, this,
            &MainWindow::appendTimestampCheckBoxToggled);
    connect(_ui.actionAddFiles, &QAction::triggered, this, [&]() {
        QList<QUrl> urls = QFileDialog::getOpenFileUrls(
            this, tr("Browse for files to add to the Backup list"));
        if(urls.count())
            _ui.backupListWidget->addItemsWithUrls(urls);
    });
    connect(_ui.actionAddDirectory, &QAction::triggered, this, [&]() {
        QUrl url = QFileDialog::getExistingDirectoryUrl(
            this, tr("Browse for directory to add to the Backup list"));
        if(!url.isEmpty())
            _ui.backupListWidget->addItemWithUrl(url);
    });
    connect(_ui.backupListWidget, &BackupListWidget::itemWithUrlAdded,
            &_filePickerDialog, &FilePickerDialog::selectUrl);
    connect(this, &MainWindow::morphBackupIntoJob, this,
            &MainWindow::createNewJob);

    // Settings pane
    loadSettings();

    // Archives pane
    connect(this, &MainWindow::archiveList, &_archivesTabWidget,
            &ArchivesTabWidget::archiveList);
    connect(this, &MainWindow::addArchive, &_archivesTabWidget,
            &ArchivesTabWidget::addArchive);

    connect(&_archivesTabWidget, &ArchivesTabWidget::jobClicked,
            _ui.jobListWidget, &JobListWidget::inspectJobByRef);
    connect(&_archivesTabWidget, &ArchivesTabWidget::displayJobDetails,
            _ui.jobListWidget, &JobListWidget::inspectJobByRef);

    connect(_ui.actionRefresh, &QAction::triggered, this,
            &MainWindow::getArchives);

    // Jobs pane
    _ui.jobListWidget->addAction(_ui.actionJobBackup);
    _ui.jobListWidget->addAction(_ui.actionJobDelete);
    _ui.jobListWidget->addAction(_ui.actionJobInspect);
    _ui.jobListWidget->addAction(_ui.actionJobRestore);
    _ui.jobListWidget->addAction(_ui.actionFilterJobs);
    _ui.jobsFilterButton->setDefaultAction(_ui.actionFilterJobs);
    connect(_ui.addJobButton, &QToolButton::clicked, this,
            &MainWindow::addJobClicked);
    connect(_ui.jobDetailsWidget, &JobWidget::collapse, this,
            &MainWindow::hideJobDetails);
    connect(_ui.jobDetailsWidget, &JobWidget::jobAdded, _ui.jobListWidget,
            &JobListWidget::addJob);
    connect(_ui.jobDetailsWidget, &JobWidget::jobAdded, this,
            &MainWindow::displayJobDetails);
    connect(_ui.jobDetailsWidget, &JobWidget::jobAdded, this,
            &MainWindow::jobAdded);
    connect(_ui.jobDetailsWidget, &JobWidget::inspectJobArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_ui.jobDetailsWidget, &JobWidget::restoreJobArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui.jobDetailsWidget, &JobWidget::deleteJobArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui.jobDetailsWidget, &JobWidget::enableSave, _ui.addJobButton,
            &QToolButton::setEnabled);
    connect(_ui.jobDetailsWidget, &JobWidget::backupJob, this,
            &MainWindow::backupJob);
    connect(_ui.jobDetailsWidget, &JobWidget::findMatchingArchives, this,
            &MainWindow::findMatchingArchives);
    connect(this, &MainWindow::matchingArchives, _ui.jobDetailsWidget,
            &JobWidget::updateMatchingArchives);
    connect(_ui.jobListWidget, &JobListWidget::displayJobDetails, this,
            &MainWindow::displayJobDetails);
    connect(_ui.jobListWidget, &JobListWidget::backupJob, this,
            &MainWindow::backupJob);
    connect(_ui.jobListWidget, &JobListWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui.jobListWidget, &JobListWidget::deleteJob, this,
            &MainWindow::deleteJob);
    connect(this, &MainWindow::jobList, _ui.jobListWidget,
            &JobListWidget::setJobs);
    connect(_ui.jobListWidget, &JobListWidget::customContextMenuRequested, this,
            &MainWindow::showJobsListMenu);
    connect(_ui.actionJobBackup, &QAction::triggered, _ui.jobListWidget,
            &JobListWidget::backupSelectedItems);
    connect(_ui.actionJobDelete, &QAction::triggered, _ui.jobListWidget,
            &JobListWidget::deleteSelectedItem);
    connect(_ui.actionJobRestore, &QAction::triggered, _ui.jobListWidget,
            &JobListWidget::restoreSelectedItem);
    connect(_ui.actionJobInspect, &QAction::triggered, _ui.jobListWidget,
            &JobListWidget::inspectSelectedItem);
    connect(_ui.sureButton, &QPushButton::clicked, this,
            &MainWindow::addDefaultJobs);

    _ui.jobListWidget->addAction(_ui.actionJobBackup);
    _ui.jobListWidget->addAction(_ui.actionAddJob);
    connect(_ui.actionAddJob, &QAction::triggered, this,
            &MainWindow::addJobClicked);
    QMenu *addJobMenu = new QMenu(_ui.addJobButton);
    addJobMenu->addAction(_ui.actionBackupAllJobs);
    connect(_ui.actionBackupAllJobs, &QAction::triggered, _ui.jobListWidget,
            &JobListWidget::backupAllJobs);
    _ui.addJobButton->setMenu(addJobMenu);

    // lambda slots for misc UI updates
    connect(_ui.backupNameLineEdit, &QLineEdit::textChanged,
            [&](const QString text) {
                if(text.isEmpty())
                    _ui.appendTimestampCheckBox->setChecked(false);
                validateBackupTab();
            });
    connect(_ui.dismissButton, &QPushButton::clicked, [&]() {
        TSettings settings;
        settings.setValue("app/default_jobs_dismissed", true);
        _ui.defaultJobs->hide();
        _ui.addJobButton->show();
    });
    connect(_ui.actionFilterJobs, &QAction::triggered, [&]() {
        _ui.jobsFilterFrame->setVisible(!_ui.jobsFilterFrame->isVisible());
        if(_ui.jobsFilter->isVisible())
            _ui.jobsFilter->setFocus();
        else
            _ui.jobsFilter->clearEditText();
    });
    connect(_ui.jobsFilter, &QComboBox::editTextChanged, _ui.jobListWidget,
            &JobListWidget::setFilter);
    connect(_ui.jobsFilter, static_cast<void (QComboBox::*)(int)>(
                                &QComboBox::currentIndexChanged),
            this, [&]() { _ui.jobListWidget->setFocus(); });
    connect(_ui.jobListWidget, &JobListWidget::countChanged, this,
            [&](int total, int visible) {
                _ui.jobsCountLabel->setText(
                    tr("Jobs (%1/%2)").arg(visible).arg(total));
            });
    connect(_ui.actionShowJobsTabHeader, &QAction::triggered,
            [&](bool checked) { _ui.jobsHeader->setVisible(checked); });

    _consoleLog = _helpWidget.getConsoleLog();
}

MainWindow::~MainWindow()
{
    commitSettings();
}

void MainWindow::loadSettings()
{
    TSettings settings;

    _ui.actionShowJobsTabHeader->setChecked(
        settings.value("app/jobs_header_enabled", true).toBool());
    _ui.jobsHeader->setVisible(_ui.actionShowJobsTabHeader->isChecked());

    if(settings.value("app/default_jobs_dismissed", false).toBool())
    {
        _ui.defaultJobs->hide();
        _ui.addJobButton->show();
    }
    else
    {
        _ui.defaultJobs->show();
        _ui.addJobButton->hide();
    }

    _ui.simulationIcon->setVisible(
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
        if(_ui.mainTabWidget->currentWidget() == _ui.jobsTab)
        {
            if(_ui.jobDetailsWidget->isVisible())
            {
                hideJobDetails();
                return;
            }
            if(_ui.jobsFilter->isVisible())
            {
                if(_ui.jobsFilter->currentText().isEmpty())
                {
                    _ui.actionFilterJobs->trigger();
                }
                else
                {
                    _ui.jobsFilter->clearEditText();
                    _ui.jobsFilter->setFocus();
                }
                return;
            }
        }
        if(_ui.journalLog->isVisible())
        {
            _ui.actionShowJournal->toggle();
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
        _ui.retranslateUi(this);
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
    connect(actionSettings, &QAction::triggered, _ui.actionGoSettings,
            &QAction::trigger);
    QMenu *appMenu = _menuBar->addMenu("");
    appMenu->addAction(actionAbout);
    appMenu->addAction(actionSettings);
    QMenu *backupMenu = _menuBar->addMenu(tr("&Backup"));
    backupMenu->addAction(_ui.actionBrowseItems);
    backupMenu->addAction(_ui.actionAddFiles);
    backupMenu->addAction(_ui.actionAddDirectory);
    backupMenu->addAction(_ui.actionClearList);
    backupMenu->addSeparator();
    backupMenu->addAction(_ui.actionBackupNow);
    backupMenu->addAction(_ui.actionCreateJob);
    QMenu *archivesMenu = _menuBar->addMenu(tr("&Archives"));
    archivesMenu->addAction(_ui.actionInspect);
    archivesMenu->addAction(_ui.actionRestore);
    archivesMenu->addAction(_ui.actionDelete);
    archivesMenu->addSeparator();
    archivesMenu->addAction(_ui.actionRefresh);
    archivesMenu->addAction(_ui.actionFilterArchives);
    QMenu *jobsMenu = _menuBar->addMenu(tr("&Jobs"));
    jobsMenu->addAction(_ui.actionJobInspect);
    jobsMenu->addAction(_ui.actionJobRestore);
    jobsMenu->addAction(_ui.actionJobDelete);
    jobsMenu->addSeparator();
    jobsMenu->addAction(_ui.actionJobBackup);
    jobsMenu->addAction(_ui.actionBackupAllJobs);
    jobsMenu->addAction(_ui.actionAddJob);
    jobsMenu->addAction(_ui.actionFilterJobs);
    QMenu *settingsMenu = _menuBar->addMenu(tr("&Settings"));
    settingsMenu->addAction(_ui.actionRefreshAccount);
    settingsMenu->addAction(_ui.actionStopTasks);
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
    windowMenu->addAction(_ui.actionGoBackup);
    windowMenu->addAction(_ui.actionGoArchives);
    windowMenu->addAction(_ui.actionGoJobs);
    windowMenu->addAction(_ui.actionGoSettings);
    windowMenu->addAction(_ui.actionGoHelp);
    windowMenu->addAction(_ui.actionShowJournal);
    windowMenu->addAction(_ui.actionShowArchivesTabHeader);
    windowMenu->addAction(_ui.actionShowJobsTabHeader);

    QMenu *  helpMenu             = _menuBar->addMenu(tr("&Help"));
    QAction *actionTarsnapWebsite = new QAction(tr("Tarsnap Website"), this);
    connect(actionTarsnapWebsite, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl("https://www.tarsnap.com"));
    });
    helpMenu->addAction(actionTarsnapWebsite);

    connect(_ui.mainTabWidget, &QTabWidget::currentChanged, this,
            &MainWindow::mainTabChanged);
    mainTabChanged(_ui.mainTabWidget->currentIndex());
}

void MainWindow::updateLoadingAnimation(bool idle)
{
    if(idle)
        _ui.busyWidget->stop();
    else
        _ui.busyWidget->animate();
}

void MainWindow::createJobClicked()
{
    emit morphBackupIntoJob(_ui.backupListWidget->itemUrls(),
                            _ui.backupNameLineEdit->text());
}

void MainWindow::createNewJob(QList<QUrl> urls, QString name)
{
    JobPtr job(new Job());
    job->setUrls(urls);
    job->setName(name);
    displayJobDetails(job);
    _ui.addJobButton->setEnabled(true);
    _ui.addJobButton->setText(tr("Save"));
    _ui.addJobButton->setProperty("save", true);
}

void MainWindow::mainTabChanged(int index)
{
    Q_UNUSED(index)
    if(_ui.mainTabWidget->currentWidget() == _ui.backupTab)
    {
        _ui.actionBrowseItems->setEnabled(true);
        validateBackupTab();
    }
    else
    {
        _ui.actionBrowseItems->setEnabled(false);
        _ui.actionBackupNow->setEnabled(false);
        _ui.actionCreateJob->setEnabled(false);
    }
    if(_ui.mainTabWidget->currentWidget() == _ui.archivesTab)
    {
        _ui.actionInspect->setEnabled(true);
        _ui.actionRestore->setEnabled(true);
        _ui.actionDelete->setEnabled(true);
        _ui.actionFilterArchives->setEnabled(true);
    }
    else
    {
        _ui.actionInspect->setEnabled(false);
        _ui.actionRestore->setEnabled(false);
        _ui.actionDelete->setEnabled(false);
        _ui.actionFilterArchives->setEnabled(false);
    }
    if(_ui.mainTabWidget->currentWidget() == _ui.jobsTab)
    {
        _ui.actionJobBackup->setEnabled(true);
        _ui.actionJobInspect->setEnabled(true);
        _ui.actionJobRestore->setEnabled(true);
        _ui.actionJobDelete->setEnabled(true);
        _ui.actionFilterJobs->setEnabled(true);
    }
    else
    {
        _ui.actionJobBackup->setEnabled(false);
        _ui.actionJobInspect->setEnabled(false);
        _ui.actionJobRestore->setEnabled(false);
        _ui.actionJobDelete->setEnabled(false);
        _ui.actionFilterJobs->setEnabled(false);
    }
}

void MainWindow::validateBackupTab()
{

    if(!_ui.backupNameLineEdit->text().isEmpty()
       && (_ui.backupListWidget->count() > 0))
    {
        _ui.actionBackupNow->setEnabled(true);
        _ui.actionCreateJob->setEnabled(true);
    }
    else
    {
        _ui.actionBackupNow->setEnabled(false);
        _ui.actionCreateJob->setEnabled(false);
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
        _ui.backupDetailLabel->setText(
            tr("%1 %2 (%3)")
                .arg(count)
                .arg(count == 1 ? tr("item") : tr("items"))
                .arg(Utils::humanBytes(size)));
    }
    else
    {
        _ui.backupDetailLabel->clear();
    }
    validateBackupTab();
}

void MainWindow::displayInspectArchive(ArchivePtr archive)
{
    displayTab(_ui.archivesTab);
    _archivesTabWidget.displayInspectArchive(archive);
}

void MainWindow::appendTimestampCheckBoxToggled(bool checked)
{
    if(checked)
    {
        QString text = _ui.backupNameLineEdit->text();
        _lastTimestamp.clear();
        _lastTimestamp.append(
            QDateTime::currentDateTime().toString(ARCHIVE_TIMESTAMP_FORMAT));
        text.append(_lastTimestamp);
        _ui.backupNameLineEdit->setText(text);
        _ui.backupNameLineEdit->setCursorPosition(0);
    }
    else
    {
        QString text = _ui.backupNameLineEdit->text();
        if(!_lastTimestamp.isEmpty() && text.endsWith(_lastTimestamp))
        {
            text.chop(_lastTimestamp.length());
            _ui.backupNameLineEdit->setText(text);
        }
    }
}

void MainWindow::backupButtonClicked()
{
    QList<QUrl> urls;
    for(int i = 0; i < _ui.backupListWidget->count(); ++i)
        urls << static_cast<BackupListWidgetItem *>(_ui.backupListWidget->item(i))
                    ->url();

    BackupTaskPtr backup(new BackupTask);
    backup->setName(_ui.backupNameLineEdit->text());
    backup->setUrls(urls);
    emit backupNow(backup);
    _ui.appendTimestampCheckBox->setChecked(false);
}

void MainWindow::updateStatusMessage(QString message, QString detail)
{
    _ui.statusBarLabel->setText(message);
    if(!detail.isEmpty())
        _ui.statusBarLabel->setToolTip(detail);
}

void MainWindow::commitSettings()
{
    TSettings settings;
    settings.setValue("app/window_geometry", saveGeometry());
    settings.setValue("app/archives_header_enabled",
                      _ui.actionShowArchivesTabHeader->isChecked());
    settings.setValue("app/jobs_header_enabled",
                      _ui.actionShowJobsTabHeader->isChecked());
    settings.sync();
}

void MainWindow::appendToJournalLog(LogEntry log)
{
    QTextCursor cursor(_ui.journalLog->document());
    if(!_ui.journalLog->document()->isEmpty())
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertBlock();
        cursor.movePosition(QTextCursor::NextBlock);
    }
    QColor bgcolor;
    int    blockCount = _ui.journalLog->document()->blockCount();
    if(blockCount % 2)
        bgcolor = qApp->palette().base().color();
    else
        bgcolor = qApp->palette().alternateBase().color();
    QTextBlockFormat bf;
    bf.setBackground(QBrush(bgcolor));
    cursor.mergeBlockFormat(bf);
    cursor.insertText(QString("[%1] %2")
                          .arg(log.timestamp.toString(Qt::DefaultLocaleShortDate))
                          .arg(log.message));
    _ui.journalLog->moveCursor(QTextCursor::End);
    _ui.journalLog->ensureCursorVisible();
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
    cursor.insertText(QString("[%1] %2\n")
                          .arg(QDateTime::currentDateTime().toString(
                              Qt::DefaultLocaleShortDate))
                          .arg(log));
    _consoleLog->moveCursor(QTextCursor::End);
    _consoleLog->ensureCursorVisible();
}

void MainWindow::setJournal(QVector<LogEntry> _log)
{
    _ui.journalLog->clear();
    foreach(LogEntry entry, _log)
        appendToJournalLog(entry);
}

void MainWindow::backupJob(JobPtr job)
{
    if(!job)
        return;

    if(!job->validateUrls())
    {
        if(job->urls().isEmpty())
        {
            QMessageBox::warning(this, tr("Job error"),
                                 tr("Job %1 has no backup paths selected. "
                                    "Nothing to back up.")
                                     .arg(job->name()));
            return;
        }
        else
        {
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, tr("Job warning"),
                tr("Some backup paths for Job %1 are not"
                   " accessible anymore and thus backup may"
                   " be incomplete."
                   " Proceed with backup?")
                    .arg(job->name()));
            if(confirm != QMessageBox::Yes)
                return;
        }
    }
    emit backupNow(job->createBackupTask());
}

void MainWindow::browseForBackupItems()
{
    displayTab(_ui.backupTab);

    _filePickerDialog.setSelectedUrls(_ui.backupListWidget->itemUrls());
    if(_filePickerDialog.exec())
        _ui.backupListWidget->setItemsWithUrls(
            _filePickerDialog.getSelectedUrls());
}

void MainWindow::displayJobDetails(JobPtr job)
{
    if(!job)
        return;

    _ui.jobListWidget->selectJob(job);
    hideJobDetails();
    _ui.jobDetailsWidget->setJob(job);
    _ui.jobDetailsWidget->show();

    displayTab(_ui.jobsTab);
}

void MainWindow::hideJobDetails()
{
    _ui.jobDetailsWidget->hide();
    if(_ui.addJobButton->property("save").toBool())
    {
        _ui.addJobButton->setText(tr("Add job"));
        _ui.addJobButton->setProperty("save", false);
        _ui.addJobButton->setEnabled(true);
    }
}

void MainWindow::addJobClicked()
{
    if(!_ui.addJobButton->isEnabled())
        return;

    if(_ui.addJobButton->property("save").toBool())
    {
        _ui.jobDetailsWidget->saveNew();
        _ui.addJobButton->setText(tr("Add job"));
        _ui.addJobButton->setProperty("save", false);
        _ui.addJobButton->setEnabled(true);
    }
    else
    {
        JobPtr job(new Job());
        displayJobDetails(job);
        _ui.addJobButton->setEnabled(false);
        _ui.addJobButton->setText(tr("Save"));
        _ui.addJobButton->setProperty("save", true);
    }
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

void MainWindow::showJobsListMenu(const QPoint &pos)
{
    QPoint globalPos = _ui.jobListWidget->viewport()->mapToGlobal(pos);
    QMenu  jobListMenu(_ui.jobListWidget);
    if(!_ui.jobListWidget->selectedItems().isEmpty())
    {
        jobListMenu.addAction(_ui.actionJobBackup);
        if(_ui.jobListWidget->selectedItems().count() == 1)
        {
            jobListMenu.addAction(_ui.actionJobInspect);
            jobListMenu.addAction(_ui.actionJobRestore);
            jobListMenu.addAction(_ui.actionJobDelete);
        }
    }
    else if(_ui.jobListWidget->count() != 0)
    {
        jobListMenu.addAction(_ui.actionBackupAllJobs);
    }
    jobListMenu.exec(globalPos);
}

void MainWindow::addDefaultJobs()
{
    TSettings settings;
    foreach(QString path, DEFAULT_JOBS)
    {
        QDir dir(QDir::home());
        if(dir.cd(path))
        {
            JobPtr job(new Job());
            job->setName(dir.dirName());
            QList<QUrl> urls;
            urls << QUrl::fromUserInput(dir.canonicalPath());
            job->setUrls(urls);
            job->save();
            _ui.jobDetailsWidget->jobAdded(job);
        }
    }
    settings.setValue("app/default_jobs_dismissed", true);
    _ui.defaultJobs->hide();
    _ui.addJobButton->show();
}

void MainWindow::updateUi()
{
    // Keyboard shortcuts
    _ui.mainTabWidget->setTabToolTip(0,
                                     _ui.mainTabWidget->tabToolTip(0).arg(
                                         _ui.actionGoBackup->shortcut().toString(
                                             QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(
        1, _ui.mainTabWidget->tabToolTip(1).arg(
               _ui.actionGoArchives->shortcut().toString(
                   QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(2,
                                     _ui.mainTabWidget->tabToolTip(2).arg(
                                         _ui.actionGoJobs->shortcut().toString(
                                             QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(
        3, _ui.mainTabWidget->tabToolTip(3).arg(
               _ui.actionGoSettings->shortcut().toString(
                   QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(4,
                                     _ui.mainTabWidget->tabToolTip(4).arg(
                                         _ui.actionGoHelp->shortcut().toString(
                                             QKeySequence::NativeText)));

    _ui.actionBackupNow->setToolTip(_ui.actionBackupNow->toolTip().arg(
        _ui.actionBackupNow->shortcut().toString(QKeySequence::NativeText)));
    _ui.backupListInfoLabel->setToolTip(_ui.backupListInfoLabel->toolTip().arg(
        _ui.actionBrowseItems->shortcut().toString(QKeySequence::NativeText)));
    _ui.backupListInfoLabel->setText(_ui.backupListInfoLabel->text().arg(
        _ui.actionBrowseItems->shortcut().toString(QKeySequence::NativeText)));
    _ui.actionShowJournal->setToolTip(_ui.actionShowJournal->toolTip().arg(
        _ui.actionShowJournal->shortcut().toString(QKeySequence::NativeText)));
    _ui.busyWidget->setToolTip(_ui.busyWidget->toolTip().arg(
        _ui.actionStopTasks->shortcut().toString(QKeySequence::NativeText)));
    _ui.addJobButton->setToolTip(_ui.addJobButton->toolTip().arg(
        _ui.actionAddJob->shortcut().toString(QKeySequence::NativeText)));
    _ui.actionFilterJobs->setToolTip(_ui.actionFilterJobs->toolTip().arg(
        _ui.actionFilterJobs->shortcut().toString(QKeySequence::NativeText)));
    _ui.jobsFilter->setToolTip(_ui.jobsFilter->toolTip().arg(
        _ui.actionFilterJobs->shortcut().toString(QKeySequence::NativeText)));
    // --

    setupMenuBar();

    if(_ui.addJobButton->property("save").toBool())
        _ui.addJobButton->setText(tr("Save"));
    else
        _ui.addJobButton->setText(tr("Add job"));
}

void MainWindow::updateSimulationIcon(int state)
{
    if(state == Qt::Unchecked)
    {
        emit getArchives();
        _ui.simulationIcon->hide();
    }
    else
    {
        _ui.simulationIcon->show();
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
void MainWindow::updateTarsnapVersion(QString versionString)
{
    _settingsWidget.updateTarsnapVersion(versionString);
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
    connect(&_settingsWidget, &SettingsWidget::getTarsnapVersion, this,
            &MainWindow::getTarsnapVersion);
    connect(&_settingsWidget, &SettingsWidget::repairCache, this,
            &MainWindow::repairCache);
}

void MainWindow::displayTab(QWidget *widget)
{
    if(_ui.mainTabWidget->currentWidget() != widget)
        _ui.mainTabWidget->setCurrentWidget(widget);
}

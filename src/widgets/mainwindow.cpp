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
#include <QMenuBar>
#include <QPainter>
#include <QSettings>
#include <QSharedPointer>
#include <QShortcut>

#define PURGE_SECONDS_DELAY 8

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::MainWindow),
      _useIECPrefixes(false),
      _purgeTimerCount(0),
      _purgeCountdownWindow(this),
      _tarsnapAccount(this),
      _aboutToQuit(false)
{
    connect(&Debug::instance(), &Debug::message, this,
            &MainWindow::appendToConsoleLog);
    _ui->setupUi(this);
    _ui->backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->jobListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
#ifdef Q_OS_OSX
    new QShortcut(QKeySequence("Ctrl+M"), this, SLOT(showMinimized()));
#endif
    new QShortcut(QKeySequence("Ctrl+K"), this, SIGNAL(getTaskInfo()));

    loadSettings();

    QMenuBar menuBar;
    if(menuBar.isNativeMenuBar())
    {
        QAction *actionAbout = new QAction(this);
        actionAbout->setMenuRole(QAction::AboutRole);
        connect(actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
        QMenu *appMenu = new QMenu(this);
        appMenu->addAction(actionAbout);
        menuBar.addMenu(appMenu);
    }
    connect(_ui->aboutButton, &QPushButton::clicked, this,
            &MainWindow::showAbout);
    // --

    _ui->mainTabWidget->setCurrentWidget(_ui->backupTab);
    _ui->settingsToolbox->setCurrentWidget(_ui->settingsAccountPage);
    _ui->mainContentSplitter->setCollapsible(0, false);
    _ui->journalLog->hide();
    _ui->archiveDetailsWidget->hide();
    _ui->jobDetailsWidget->hide();
    _ui->outOfDateNoticeLabel->hide();
    _ui->archivesFilter->hide();
    _ui->jobsFilter->hide();
#if(QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    _ui->consoleLog->setPlaceholderText(tr("No events yet"));
    _ui->journalLog->setPlaceholderText(tr("No messages yet"));
#endif

    // Purge widget setup
    _purgeCountdownWindow.setIcon(QMessageBox::Critical);
    _purgeCountdownWindow.setWindowTitle(
        tr("Deleting all archives: press Cancel to abort"));
    _purgeCountdownWindow.setStandardButtons(QMessageBox::Cancel);
    connect(&_purgeTimer, &QTimer::timeout, this, &MainWindow::purgeTimerFired);
    // --

    // Ui actions setup

    _ui->settingsTab->addAction(_ui->actionRefreshAccount);
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            &MainWindow::getOverallStats);
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            [&]() { _tarsnapAccount.getAccountInfo(); });
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

    // Backup pane
    _ui->backupButton->addAction(_ui->actionCreateJob);
    connect(_ui->actionCreateJob, &QAction::triggered, this,
            &MainWindow::createJobClicked);
    connect(_ui->backupListWidget, &BackupListWidget::itemTotals, this,
            &MainWindow::updateBackupItemTotals);
    _ui->backupListWidget->addAction(_ui->actionClearList);
    connect(_ui->actionClearList, &QAction::triggered, _ui->backupListWidget,
            &BackupListWidget::clear);
    _ui->backupListWidget->addAction(_ui->actionBrowseItems);
    connect(_ui->actionBrowseItems, &QAction::triggered, this,
            &MainWindow::browseForBackupItems);
    connect(_ui->backupListInfoLabel, &QLabel::linkActivated, this,
            &MainWindow::browseForBackupItems);
    connect(_ui->backupButton, &QPushButton::clicked, this,
            &MainWindow::backupButtonClicked);
    connect(_ui->appendTimestampCheckBox, &QCheckBox::toggled, this,
            &MainWindow::appendTimestampCheckBoxToggled);

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
    connect(_ui->iecPrefixesCheckBox, &QCheckBox::toggled, this,
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
    connect(_ui->limitUploadSpinBox, &QSpinBox::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui->limitDownloadSpinBox, &QSpinBox::editingFinished, this,
            &MainWindow::commitSettings);

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
    connect(&_tarsnapAccount, &TarsnapAccount::accountCredit, this,
            &MainWindow::updateAccountCredit);
    connect(&_tarsnapAccount, &TarsnapAccount::getKeyId, this,
            &MainWindow::getKeyId);
    connect(_ui->updateAccountButton, &QPushButton::clicked,
            _ui->actionRefreshAccount, &QAction::trigger);
    connect(&_tarsnapAccount, &TarsnapAccount::lastMachineActivity, this,
            &MainWindow::updateLastMachineActivity);
    connect(_ui->accountActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(true, false); });
    connect(_ui->machineActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(false, true); });

    connect(_ui->clearJournalButton, &QPushButton::clicked, this,
            &MainWindow::clearJournalClicked);

    // Archives
    _ui->archiveListWidget->addAction(_ui->actionRefresh);
    _ui->archiveListWidget->addAction(_ui->actionInspect);
    _ui->archiveListWidget->addAction(_ui->actionDelete);
    _ui->archiveListWidget->addAction(_ui->actionRestore);
    _ui->archiveListWidget->addAction(_ui->actionShowFilter);
    connect(this, &MainWindow::archiveList, _ui->archiveListWidget,
            &ArchiveListWidget::addArchives);
    connect(_ui->archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui->archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::displayJobDetails,
            _ui->jobListWidget, &JobListWidget::inspectJobByRef);
    connect(_ui->archiveDetailsWidget, &ArchiveWidget::jobClicked,
            _ui->jobListWidget, &JobListWidget::inspectJobByRef);

    connect(_ui->archiveListWidget,
            &ArchiveListWidget::customContextMenuRequested, this,
            &MainWindow::showArchiveListMenu);
    connect(_ui->actionRefresh, &QAction::triggered, this,
            &MainWindow::getArchives);
    connect(_ui->actionDelete, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::removeSelectedItems);
    connect(_ui->actionRestore, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::restoreSelectedItem);
    connect(_ui->actionInspect, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::inspectSelectedItem);

    // Jobs
    _ui->jobListWidget->addAction(_ui->actionJobBackup);
    _ui->jobListWidget->addAction(_ui->actionJobDelete);
    _ui->jobListWidget->addAction(_ui->actionJobInspect);
    _ui->jobListWidget->addAction(_ui->actionJobRestore);
    _ui->jobListWidget->addAction(_ui->actionShowFilter);
    connect(_ui->addJobButton, &QToolButton::clicked, this,
            &MainWindow::addJobClicked);
    connect(_ui->jobDetailsWidget, &JobWidget::collapse, this,
            &MainWindow::hideJobDetails);
    connect(_ui->jobDetailsWidget, &JobWidget::jobAdded, _ui->jobListWidget,
            &JobListWidget::addJob);
    connect(_ui->jobDetailsWidget, &JobWidget::jobAdded, this,
            &MainWindow::displayJobDetails);
    connect(_ui->jobDetailsWidget, &JobWidget::inspectJobArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_ui->jobDetailsWidget, &JobWidget::restoreJobArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui->jobDetailsWidget, &JobWidget::deleteJobArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui->jobDetailsWidget, &JobWidget::deleteJobArchives,
            _ui->archiveListWidget, &ArchiveListWidget::disableArchives);
    connect(_ui->jobDetailsWidget, &JobWidget::enableSave, _ui->addJobButton,
            &QToolButton::setEnabled);
    connect(_ui->jobDetailsWidget, &JobWidget::backupJob, this,
            &MainWindow::backupNow);
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
    connect(_ui->jobListWidget, &JobListWidget::customContextMenuRequested,
            this, &MainWindow::showJobsListMenu);
    connect(_ui->actionJobBackup, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::backupSelectedItems);
    connect(_ui->actionJobDelete, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::deleteSelectedItem);
    connect(_ui->actionJobRestore, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::restoreSelectedItem);
    connect(_ui->actionJobInspect, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::inspectSelectedItem);
    connect(_ui->sureButton, &QPushButton::clicked, this,
            &MainWindow::addDefaultJobs);

    _ui->jobListWidget->addAction(_ui->actionJobBackup);
    _ui->jobListWidget->addAction(_ui->actionAddJob);
    connect(_ui->actionAddJob, &QAction::triggered, this,
            &MainWindow::addJobClicked);
    QMenu *addJobMenu = new QMenu(_ui->addJobButton);
    addJobMenu->addAction(_ui->actionBackupAllJobs);
    connect(_ui->actionBackupAllJobs, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::backupAllJobs);
    _ui->addJobButton->setMenu(addJobMenu);

    // lambda slots for misc UI updates
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
    connect(_ui->downloadsDirLineEdit, &QLineEdit::textChanged, [&]() {
        QFileInfo file(_ui->downloadsDirLineEdit->text());
        if(file.exists() && file.isDir() && file.isWritable())
            _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
        else
            _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
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
        _ui->expandJournalButton->setChecked(
            !_ui->expandJournalButton->isChecked());
    });
    connect(_ui->simulationCheckBox, &QCheckBox::stateChanged, [&](int state) {
        if(state == Qt::Unchecked)
        {
            emit getArchives();
            _ui->simulationIcon->hide();
        }
        else
        {
            _ui->simulationIcon->show();
        }
    });
    connect(_ui->repairCacheButton, &QPushButton::clicked, this,
            [&]() { emit repairCache(true); });
    connect(_ui->skipSystemDefaultsButton, &QPushButton::clicked,
            [&]() { _ui->skipSystemLineEdit->setText(DEFAULT_SKIP_FILES); });
    connect(_ui->jobListWidget, &JobListWidget::deleteJob, this,
            [&](JobPtr job, bool purgeArchives) {
                if(_ui->jobDetailsWidget->job() == job)
                    hideJobDetails();
                if(purgeArchives)
                    _ui->archiveListWidget->disableArchives(job->archives());
            });
    connect(_ui->iecPrefixesCheckBox, &QCheckBox::toggled, this, [&]() {
        QMessageBox::information(this, QApplication::applicationName(),
                                 tr("The new size notation will take effect on "
                                    "application restart."));
    });
    connect(_ui->dismissButton, &QPushButton::clicked, [&]() {
        QSettings settings;
        settings.setValue("app/default_jobs_dismissed", true);
        _ui->defaultJobs->hide();
        _ui->addJobButton->show();
    });
    connect(_ui->actionShowFilter, &QAction::triggered, [&]()
    {
        if(_ui->mainTabWidget->currentWidget() == _ui->archivesTab)
        {
            _ui->archivesFilter->setVisible(!_ui->archivesFilter->isVisible());
            _ui->archivesFilter->setFocus();
        }
        else if(_ui->mainTabWidget->currentWidget() == _ui->jobsTab)
        {
            _ui->jobsFilter->setVisible(!_ui->jobsFilter->isVisible());
            _ui->jobsFilter->setFocus();
        }
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

    _ui->accountCreditLabel->setText(
        settings.value("tarsnap/credit", tr("click update button")).toString());
    _ui->machineActivity->setText(
        settings.value("tarsnap/machine_activity", tr("click update button")).toString());
    _ui->accountUserLineEdit->setText(
        settings.value("tarsnap/user", "").toString());
    _ui->accountMachineKeyLineEdit->setText(
        settings.value("tarsnap/key", "").toString());
    _ui->accountMachineLineEdit->setText(
        settings.value("tarsnap/machine", "").toString());
    _ui->tarsnapPathLineEdit->setText(
        settings.value("tarsnap/path", "").toString());
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
    _useIECPrefixes = settings.value("app/iec_prefixes", false).toBool();
    _ui->iecPrefixesCheckBox->setChecked(_useIECPrefixes);
    _ui->skipFilesSizeSpinBox->setValue(
        settings.value("app/skip_files_size", 0).toInt());
    _ui->skipSystemJunkCheckBox->setChecked(
        settings.value("app/skip_system_enabled", false).toBool());
    _ui->skipSystemLineEdit->setEnabled(_ui->skipSystemJunkCheckBox->isChecked());
    _ui->skipSystemLineEdit->setText(
        settings.value("app/skip_system_files", DEFAULT_SKIP_FILES).toString());
    _ui->skipNoDumpCheckBox->setChecked(
        settings.value("app/skip_nodump", false).toBool());
    _ui->downloadsDirLineEdit->setText(
        settings.value("app/downloads_dir", DOWNLOADS).toString());
    _ui->appDataDirLineEdit->setText(
        settings.value("app/app_data", "").toString());
    _ui->notificationsCheckBox->setChecked(
        settings.value("app/notifications", true).toBool());
    _ui->limitUploadSpinBox->setValue(
        settings.value("app/limit_upload", 0).toInt());
    _ui->limitDownloadSpinBox->setValue(
                settings.value("app/limit_download", 0).toInt());

    if(settings.value("app/default_jobs_dismissed", false).toBool())
    {
        _ui->defaultJobs->hide();
        _ui->addJobButton->show();
    }
    else
    {
        _ui->defaultJobs->show();
        _ui->addJobButton->hide();
    }
}

void MainWindow::initialize()
{
    QSettings settings;
    QDate creditDate = settings.value("tarsnap/credit_date", QDate()).toDate();
    if(creditDate.isValid())
    {
        _ui->accountCreditLabel->setToolTip(creditDate.toString());
        qint64 daysElapsed = creditDate.daysTo(QDate::currentDate());
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

    if(!validateTarsnapPath())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Tarsnap CLI utilities not found. Go to "
                                 " Settings -> Advanced page to fix that."));
    }

    if(!validateMachineKeyPath())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Machine key file not found. Go to "
                                 " Settings -> Tarsnap page to fix that."));
    }

    if(!validateTarsnapCache())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Tarsnap cache dir is invalid. Go to "
                                 " Settings -> Advanced page to fix that."));
    }

    if(!settings.value("tarsnap/dry_run", false).toBool())
        emit getArchives();
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    if(width() < 600)
    {
        QPixmap pixmap(":/icons/tarsnap-logo-icon.png");
        QIcon icon;
        icon.addFile(":/icons/tarsnap-logo-icon.png");
        icon.paint(&p, width() - pixmap.width() - 5, 2, pixmap.width(), pixmap.height());
    }
    else
    {
        QPixmap pixmap(":/icons/tarsnap-logo.png");
        QIcon icon;
        icon.addFile(":/icons/tarsnap-logo.png");
        icon.paint(&p, width() - pixmap.width() - 5, 2, pixmap.width(), pixmap.height());
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if((_ui->mainTabWidget->currentWidget() == _ui->archivesTab) &&
           (_ui->archiveDetailsWidget->isVisible()))
        {
            _ui->archiveDetailsWidget->hide();
            _ui->archiveDetailsWidget->setArchive(ArchivePtr());
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(_aboutToQuit)
    {
        qApp->setQuitLockEnabled(true);
        event->accept();
    }
    else
    {
        _aboutToQuit = true;
        emit getTaskInfo();
        event->ignore();
    }
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
        Utils::humanBytes(sizeTotal, _useIECPrefixes));
    _ui->accountTotalSizeLabel->setToolTip(tooltip);
    _ui->accountActualSizeLabel->setText(
        Utils::humanBytes(sizeUniqueCompressed, _useIECPrefixes));
    _ui->accountActualSizeLabel->setToolTip(tooltip);
    quint64 storageSaved =
        sizeTotal >= sizeUniqueCompressed ? sizeTotal - sizeUniqueCompressed : 0;
    _ui->accountStorageSavedLabel->setText(
        Utils::humanBytes(storageSaved, _useIECPrefixes));
    _ui->accountStorageSavedLabel->setToolTip(tooltip);
    _ui->accountArchivesCountLabel->setText(QString::number(archiveCount));
}

void MainWindow::updateTarsnapVersion(QString versionString)
{
    setTarsnapVersion(versionString);
    QSettings settings;
    settings.setValue("tarsnap/version", versionString);
}

void MainWindow::setTarsnapVersion(QString versionString)
{
    if(versionString.isEmpty())
    {
        _ui->tarsnapVersionLabel->clear();
        _ui->tarsnapVersionLabel->hide();
    }
    else
    {
        _ui->tarsnapVersionLabel->setText(tr("Tarsnap version ") +
                                          versionString + tr(" detected"));
        _ui->tarsnapVersionLabel->show();
    }
}

void MainWindow::createJobClicked()
{
    JobPtr job(new Job());
    job->setUrls(_ui->backupListWidget->itemUrls());
    job->setName(_ui->backupNameLineEdit->text());
    displayJobDetails(job);
    _ui->addJobButton->setEnabled(true);
    _ui->addJobButton->setText(tr("Save"));
    _ui->addJobButton->setProperty("save", true);
}

void MainWindow::showAbout()
{
    QDialog *aboutWindow = new QDialog(this);
    Ui::aboutWidget aboutUi;
    aboutUi.setupUi(aboutWindow);
    aboutUi.versionLabel->setText(tr("version ") +
                                  QCoreApplication::applicationVersion());
    aboutWindow->setAttribute(Qt::WA_DeleteOnClose, true);
    aboutWindow->setWindowFlags(
        (aboutWindow->windowFlags() | Qt::CustomizeWindowHint) &
        ~Qt::WindowMaximizeButtonHint);
    connect(aboutUi.checkUpdateButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(
            QUrl("https://github.com/Tarsnap/tarsnap-gui/releases"));
    });
    aboutWindow->show();
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
        _ui->backupDetailLabel->setText(
            tr("%1 %2 (%3)")
                .arg(count)
                .arg(count == 1 ? "item" : "items")
                .arg(Utils::humanBytes(size, _useIECPrefixes)));
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
    if(archive->sizeTotal() == 0)
        emit loadArchiveStats(archive);

    if(archive->contents().count() == 0)
        emit loadArchiveContents(archive);

    _ui->archiveListWidget->setSelectedArchive(archive);

    _ui->archiveDetailsWidget->setArchive(archive);
    if(!_ui->archiveDetailsWidget->isVisible())
        _ui->archiveDetailsWidget->show();

    if(_ui->mainTabWidget->currentWidget() != _ui->archivesTab)
        _ui->mainTabWidget->setCurrentWidget(_ui->archivesTab);
}

void MainWindow::appendTimestampCheckBoxToggled(bool checked)
{
    if(checked)
    {
        QString text = _ui->backupNameLineEdit->text();
        _lastTimestamp.clear();
        _lastTimestamp.append(
            QDateTime::currentDateTime().toString(ARCHIVE_TIMESTAMP_FORMAT));
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
    emit backupNow(backup);
    _ui->appendTimestampCheckBox->setChecked(false);
}

void MainWindow::updateStatusMessage(QString message, QString detail)
{
    _ui->statusBarLabel->setText(message);
    _ui->statusBarLabel->setToolTip(detail);
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
    settings.setValue("tarsnap/traverse_mount", _ui->traverseMountCheckBox->isChecked());
    settings.setValue("tarsnap/follow_symlinks", _ui->followSymLinksCheckBox->isChecked());
    settings.setValue("tarsnap/no_default_config", _ui->ignoreConfigCheckBox->isChecked());
    settings.setValue("tarsnap/dry_run", _ui->simulationCheckBox->isChecked());
    settings.setValue("app/iec_prefixes", _ui->iecPrefixesCheckBox->isChecked());
    settings.setValue("app/skip_files_size", _ui->skipFilesSizeSpinBox->value());
    settings.setValue("app/skip_system_enabled", _ui->skipSystemJunkCheckBox->isChecked());
    settings.setValue("app/skip_system_files", _ui->skipSystemLineEdit->text());
    settings.setValue("app/skip_nodump", _ui->skipNoDumpCheckBox->isChecked());
    settings.setValue("app/downloads_dir", _ui->downloadsDirLineEdit->text());
    settings.setValue("app/app_data", _ui->appDataDirLineEdit->text());
    settings.setValue("app/notifications", _ui->notificationsCheckBox->isChecked());
    settings.setValue("app/limit_upload", _ui->limitUploadSpinBox->value());
    settings.setValue("app/limit_download", _ui->limitDownloadSpinBox->value());
    settings.sync();
    emit settingsChanged();
}

bool MainWindow::validateMachineKeyPath()
{
    QFileInfo machineKeyFile(_ui->accountMachineKeyLineEdit->text());
    if(machineKeyFile.exists() && machineKeyFile.isFile() &&
       machineKeyFile.isReadable())
    {
        _ui->accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: black;}");
        return true;
    }
    else
    {
        _ui->accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: red;}");
        return false;
    }
}

bool MainWindow::validateTarsnapPath()
{
    if(Utils::findTarsnapClientInPath(_ui->tarsnapPathLineEdit->text()).isEmpty())
    {
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: red;}");
        setTarsnapVersion("");
        return false;
    }
    else
    {
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: black;}");
        emit getTarsnapVersion(_ui->tarsnapPathLineEdit->text());
        return true;
    }
}

bool MainWindow::validateTarsnapCache()
{
    if(Utils::validateTarsnapCache(_ui->tarsnapCacheLineEdit->text()).isEmpty())
    {
        _ui->tarsnapCacheLineEdit->setStyleSheet("QLineEdit {color: red;}");
        return false;
    }
    else
    {
        _ui->tarsnapCacheLineEdit->setStyleSheet("QLineEdit {color: black;}");
        return true;
    }
}

void MainWindow::purgeTimerFired()
{
    if(_purgeTimerCount <= 1)
    {
        _purgeTimer.stop();
        _purgeCountdownWindow.accept();
        emit purgeArchives();
    }
    else
    {
        --_purgeTimerCount;
        _purgeCountdownWindow.setText(
            tr("Purging all archives in %1 seconds...").arg(_purgeTimerCount));
    }
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
    cursor.insertText(QString("[%1] %2")
                          .arg(log.timestamp.toString(Qt::DefaultLocaleShortDate))
                          .arg(log.message));
    _ui->journalLog->moveCursor(QTextCursor::End);
    _ui->journalLog->ensureCursorVisible();
}

void MainWindow::appendToConsoleLog(const QString &log)
{
    QTextCursor cursor(_ui->consoleLog->document());
    if(!_ui->consoleLog->document()->isEmpty())
    {
        cursor.movePosition(QTextCursor::End);
        cursor.insertBlock();
        cursor.movePosition(QTextCursor::NextBlock);
    }
    QTextBlockFormat bf;
    if(cursor.blockFormat().background().color() == qApp->palette().base().color())
        bf.setBackground(QBrush(qApp->palette().alternateBase().color()));
    else
        bf.setBackground(QBrush(qApp->palette().base().color()));
    cursor.mergeBlockFormat(bf);
    cursor.insertText(QString("[%1] %2\n").arg(QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate)).arg(log));
    cursor.insertText(QLatin1String("----------------------------------------------------------------------------"));
    _ui->consoleLog->moveCursor(QTextCursor::End);
    _ui->consoleLog->ensureCursorVisible();
}

void MainWindow::setJournal(QVector<LogEntry> _log)
{
    _ui->journalLog->clear();
    foreach(LogEntry entry, _log)
        appendToJournalLog(entry);
}

void MainWindow::saveKeyId(QString key, int id)
{
    if(key == _ui->accountMachineKeyLineEdit->text())
    {
        QSettings settings;
        settings.setValue("tarsnap/key_id", id);
        settings.sync();
    }
}

void MainWindow::browseForBackupItems()
{
    FilePickerDialog picker;
    connect(_ui->backupListWidget, &BackupListWidget::itemWithUrlAdded,
            &picker, &FilePickerDialog::selectUrl);
    picker.setSelectedUrls(_ui->backupListWidget->itemUrls());
    if(picker.exec())
        _ui->backupListWidget->setItemsWithUrls(picker.getSelectedUrls());
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
    QString downDir =
        QFileDialog::getExistingDirectory(this,
                                          tr("Browse for downloads directory"),
                                          DOWNLOADS);
    if(!downDir.isEmpty())
    {
        _ui->downloadsDirLineEdit->setText(downDir);
        commitSettings();
    }
}

void MainWindow::displayJobDetails(JobPtr job)
{
    if(!job)
        return;

    _ui->jobListWidget->selectJob(job);
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

void MainWindow::displayStopTasks(bool backupTaskRunning, int runningTasks,
                                  int queuedTasks)
{
    if(!runningTasks && !queuedTasks)
    {
        if(_aboutToQuit)
        {
            close();
            return;
        }
        else
        {
            QMessageBox::information(this, tr("Stop Tasks"),
                                     tr("There are no running or queued tasks."));
            return;
        }
    }

    QMessageBox msgBox;
    msgBox.setText(tr("There are %1 running tasks and %2 queued.")
                       .arg(runningTasks)
                       .arg(queuedTasks));
    msgBox.setInformativeText(tr("What do you want to do?"));
    QPushButton *interruptBackup = nullptr;
    if(backupTaskRunning)
        interruptBackup =
            msgBox.addButton(tr("Interrupt backup"), QMessageBox::ActionRole);
    QPushButton *stopRunning = nullptr;
    if(runningTasks && !_aboutToQuit)
        stopRunning =
            msgBox.addButton(tr("Stop running"), QMessageBox::ActionRole);
    QPushButton *stopQueued = nullptr;
    if(queuedTasks && !_aboutToQuit)
        stopQueued =
            msgBox.addButton(tr("Cancel queued"), QMessageBox::ActionRole);
    QPushButton *stopAll = nullptr;
    if(runningTasks || queuedTasks)
        stopAll = msgBox.addButton(tr("Stop all"), QMessageBox::ActionRole);
    QPushButton *background = nullptr;
    if((runningTasks || queuedTasks) && _aboutToQuit)
        background = msgBox.addButton(tr("Proceed in background"), QMessageBox::ActionRole);
    QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(cancel);
    msgBox.exec();
    if(msgBox.clickedButton() == interruptBackup)
    {
        emit stopTasks(true, false, true);
        updateStatusMessage("Interrupting current backup.");
    }
    if(msgBox.clickedButton() == stopQueued)
    {
        emit stopTasks(false, false, true);
        updateStatusMessage("Cleared queued tasks.");
    }
    else if(msgBox.clickedButton() == stopRunning)
    {
        emit stopTasks(false, true, false);
        updateStatusMessage("Stopped running tasks.");
    }
    else if(msgBox.clickedButton() == stopAll)
    {
        emit stopTasks(false, true, true);
        updateStatusMessage("Stopped running tasks and cleared queued ones.");
    }
    else if((msgBox.clickedButton() == cancel) && _aboutToQuit)
    {
        _aboutToQuit = false;
    }

    if(_aboutToQuit)
        close();
}

void MainWindow::tarsnapError(TarsnapError error)
{
    switch(error)
    {
    case TarsnapError::CacheError:
    {
        auto confirm =
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
                                 " Advanced."));
        break;
    }
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
    if(activityFields.size() < 2)
        return;
    QString machineActivity = activityFields[0] + ' ' + activityFields[1];
    QSettings settings;
    settings.setValue("tarsnap/machine_activity", machineActivity);
    _ui->machineActivity->setText(machineActivity);
    _ui->machineActivity->setToolTip(activityFields.join(' '));
    _ui->machineActivity->resize(_ui->machineActivity->fontMetrics()
                                 .width(_ui->machineActivity->text()),
                                 _ui->machineActivity->sizeHint().height());
}

void MainWindow::clearJournalClicked()
{
    auto confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Clear journal log? All entries will "
                                 "be deleted forever."));
    if(confirm == QMessageBox::Yes)
        emit clearJournal();
}

void MainWindow::showArchiveListMenu(const QPoint &pos)
{
    QPoint globalPos = _ui->archiveListWidget->viewport()->mapToGlobal(pos);
    QMenu  archiveListMenu(_ui->archiveListWidget);
    archiveListMenu.addAction(_ui->actionRefresh);
    if(!_ui->archiveListWidget->selectedItems().isEmpty())
    {
        if(_ui->archiveListWidget->selectedItems().count() == 1)
        {
            archiveListMenu.addAction(_ui->actionInspect);
            archiveListMenu.addAction(_ui->actionRestore);
        }
        archiveListMenu.addAction(_ui->actionDelete);
    }
    archiveListMenu.exec(globalPos);
}

void MainWindow::showJobsListMenu(const QPoint &pos)
{
    QPoint globalPos = _ui->jobListWidget->viewport()->mapToGlobal(pos);
    QMenu  jobListMenu(_ui->jobListWidget);
    if(!_ui->jobListWidget->selectedItems().isEmpty())
    {
        jobListMenu.addAction(_ui->actionJobBackup);
        if(_ui->jobListWidget->selectedItems().count() == 1)
        {
            jobListMenu.addAction(_ui->actionJobInspect);
            jobListMenu.addAction(_ui->actionJobRestore);
            jobListMenu.addAction(_ui->actionJobDelete);
        }
    }
    jobListMenu.exec(globalPos);
}

void MainWindow::addDefaultJobs()
{
    QSettings settings;
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
            job->setOptionScheduledEnabled(false);
            job->setOptionPreservePaths(settings.value("tarsnap/preserve_pathnames", true).toBool());
            job->setOptionTraverseMount(settings.value("tarsnap/traverse_mount", true).toBool());
            job->setOptionFollowSymLinks(settings.value("tarsnap/follow_symlinks", false).toBool());
            job->setOptionSkipNoDump(settings.value("app/skip_nodump", false).toBool());
            job->setOptionSkipFilesSize(settings.value("app/skip_files_size", 0).toInt());
            job->setOptionSkipFiles(settings.value("app/skip_system_enabled", false).toBool());
            job->setOptionSkipFilesPatterns(settings.value("app/skip_system_files", DEFAULT_SKIP_FILES).toString());
            job->save();
            _ui->jobDetailsWidget->jobAdded(job);
        }
    }
    settings.setValue("app/default_jobs_dismissed", true);
    _ui->defaultJobs->hide();
    _ui->addJobButton->show();
}

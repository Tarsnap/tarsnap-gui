#include "mainwindow.h"
#include "backuplistwidgetitem.h"
#include "debug.h"
#include "filepickerdialog.h"
#include "ui_aboutwidget.h"
#include "utils.h"
#include "translator.h"

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

#define PURGE_SECONDS_DELAY 8

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      _menuBar(nullptr),
      _purgeTimerCount(0),
      _purgeCountdown(this),
      _tarsnapAccount(this),
      _aboutToQuit(false)
{
    connect(&Debug::instance(), &Debug::message, this,
            &MainWindow::appendToConsoleLog);

    // Ui initialization
    _ui.setupUi(this);
    _ui.backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui.archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui.jobListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    _ui.mainTabWidget->setCurrentWidget(_ui.backupTab);
    validateBackupTab();
    _ui.settingsToolbox->setCurrentWidget(_ui.settingsAccountPage);
    _ui.mainContentSplitter->setCollapsible(0, false);
    _ui.journalLog->hide();
    _ui.archiveDetailsWidget->hide();
    _ui.jobDetailsWidget->hide();
    _ui.outOfDateNoticeLabel->hide();
    _ui.archivesFilterFrame->hide();
    _ui.jobsFilterFrame->hide();

#ifdef Q_OS_OSX
    _ui.aboutButton->hide();
#endif
    // --

    updateUi();

    // Purge widget setup
    _purgeCountdown.setIcon(QMessageBox::Critical);
    _purgeCountdown.setStandardButtons(QMessageBox::Cancel);
    connect(&_purgeTimer, &QTimer::timeout, this, &MainWindow::purgeTimerFired);
    // --

    // Ui actions setup
    _ui.settingsTab->addAction(_ui.actionRefreshAccount);
    connect(_ui.actionRefreshAccount, &QAction::triggered, this,
            &MainWindow::getOverallStats);
    connect(_ui.actionRefreshAccount, &QAction::triggered, this,
            [&]() { _tarsnapAccount.getAccountInfo(); });
    addAction(_ui.actionGoBackup);
    addAction(_ui.actionGoArchives);
    addAction(_ui.actionGoJobs);
    addAction(_ui.actionGoSettings);
    addAction(_ui.actionGoHelp);
    connect(_ui.actionGoBackup, &QAction::triggered,
            [&]() { _ui.mainTabWidget->setCurrentWidget(_ui.backupTab); });
    connect(_ui.actionGoArchives, &QAction::triggered,
            [&]() { _ui.mainTabWidget->setCurrentWidget(_ui.archivesTab); });
    connect(_ui.actionGoJobs, &QAction::triggered,
            [&]() { _ui.mainTabWidget->setCurrentWidget(_ui.jobsTab); });
    connect(_ui.actionGoSettings, &QAction::triggered,
            [&]() { _ui.mainTabWidget->setCurrentWidget(_ui.settingsTab); });
    connect(_ui.actionGoHelp, &QAction::triggered,
            [&]() { _ui.mainTabWidget->setCurrentWidget(_ui.helpTab); });
    addAction(_ui.actionShowJournal);
    _ui.expandJournalButton->setDefaultAction(_ui.actionShowJournal);
    connect(_ui.actionShowJournal, &QAction::toggled, _ui.journalLog, &QWidget::setVisible);
    connect(_ui.statusBarLabel, &ElidedLabel::clicked, _ui.actionShowJournal, &QAction::toggle);
    addAction(_ui.actionStopTasks);
    connect(_ui.actionStopTasks, &QAction::triggered, this, &MainWindow::getTaskInfo);
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
    connect(_ui.backupListInfoLabel, &QLabel::linkActivated, _ui.actionBrowseItems,
            &QAction::trigger);
    connect(_ui.appendTimestampCheckBox, &QCheckBox::toggled, this,
            &MainWindow::appendTimestampCheckBoxToggled);
    connect(_ui.actionAddFiles, &QAction::triggered, this, [&]()
    {
        QList<QUrl> urls = QFileDialog::getOpenFileUrls(this,
                              tr("Browse for files to add to the Backup list"));
        if(urls.count())
            _ui.backupListWidget->addItemsWithUrls(urls);
    });
    connect(_ui.actionAddDirectory, &QAction::triggered, this, [&]()
    {
        QUrl url = QFileDialog::getExistingDirectoryUrl(this,
                          tr("Browse for directory to add to the Backup list"));
        if(!url.isEmpty())
            _ui.backupListWidget->addItemWithUrl(url);
    });

    // Settings pane
    loadSettings();
    connect(_ui.aboutButton, &QPushButton::clicked, this,
            &MainWindow::showAbout);
    connect(_ui.accountUserLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.accountMachineLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.accountMachineKeyLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.tarsnapPathLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.tarsnapCacheLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.aggressiveNetworkingCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.accountMachineKeyLineEdit, &QLineEdit::textChanged, this,
            &MainWindow::validateMachineKeyPath);
    connect(_ui.tarsnapPathLineEdit, &QLineEdit::textChanged, this,
            &MainWindow::validateTarsnapPath);
    connect(_ui.tarsnapCacheLineEdit, &QLineEdit::textChanged, this,
            &MainWindow::validateTarsnapCache);
    connect(_ui.appDataDirLineEdit, &QLineEdit::textChanged, this,
            &MainWindow::validateAppDataDir);
    connect(_ui.iecPrefixesCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.notificationsCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.preservePathsCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.downloadsDirLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.traverseMountCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.followSymLinksCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.skipFilesSizeSpinBox, &QSpinBox::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.skipSystemJunkCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.skipSystemLineEdit, &QLineEdit::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.skipNoDumpCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.simulationCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.ignoreConfigCheckBox, &QCheckBox::toggled, this,
            &MainWindow::commitSettings);
    connect(_ui.limitUploadSpinBox, &QSpinBox::editingFinished, this,
            &MainWindow::commitSettings);
    connect(_ui.limitDownloadSpinBox, &QSpinBox::editingFinished, this,
            &MainWindow::commitSettings);

    connect(_ui.accountMachineUseHostnameButton, &QPushButton::clicked, this,
            &MainWindow::accountMachineUseHostnameButtonClicked);
    connect(_ui.accountMachineKeyBrowseButton, &QPushButton::clicked, this,
            &MainWindow::accountMachineKeyBrowseButtonClicked);
    connect(_ui.tarsnapPathBrowseButton, &QPushButton::clicked, this,
            &MainWindow::tarsnapPathBrowseButtonClicked);
    connect(_ui.tarsnapCacheBrowseButton, &QPushButton::clicked, this,
            &MainWindow::tarsnapCacheBrowseButton);
    connect(_ui.appDataDirBrowseButton, &QPushButton::clicked, this,
            &MainWindow::appDataButtonClicked);
    connect(_ui.purgeArchivesButton, &QPushButton::clicked, this,
            &MainWindow::purgeArchivesButtonClicked);
    connect(_ui.runSetupWizard, &QPushButton::clicked, this,
            &MainWindow::runSetupWizardClicked);
    connect(_ui.downloadsDirBrowseButton, &QPushButton::clicked, this,
            &MainWindow::downloadsDirBrowseButtonClicked);
    connect(&_tarsnapAccount, &TarsnapAccount::accountCredit, this,
            &MainWindow::updateAccountCredit);
    connect(&_tarsnapAccount, &TarsnapAccount::getKeyId, this,
            &MainWindow::getKeyId);
    connect(_ui.updateAccountButton, &QPushButton::clicked,
            _ui.actionRefreshAccount, &QAction::trigger);
    connect(&_tarsnapAccount, &TarsnapAccount::lastMachineActivity, this,
            &MainWindow::updateLastMachineActivity);
    connect(_ui.accountActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(true, false); });
    connect(_ui.machineActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(false, true); });
    connect(_ui.clearJournalButton, &QPushButton::clicked, this,
            &MainWindow::clearJournalClicked);

    // Archives pane
    _ui.archiveListWidget->addAction(_ui.actionRefresh);
    _ui.archiveListWidget->addAction(_ui.actionInspect);
    _ui.archiveListWidget->addAction(_ui.actionDelete);
    _ui.archiveListWidget->addAction(_ui.actionRestore);
    _ui.archiveListWidget->addAction(_ui.actionFilterArchives);
    _ui.archivesFilterButton->setDefaultAction(_ui.actionFilterArchives);
    connect(this, &MainWindow::archiveList, _ui.archiveListWidget,
            &ArchiveListWidget::setArchives);
    connect(this, &MainWindow::addArchive, _ui.archiveListWidget,
            &ArchiveListWidget::addArchive);
    connect(_ui.archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &MainWindow::displayInspectArchive);
    connect(_ui.archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &MainWindow::deleteArchives);
    connect(_ui.archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &MainWindow::restoreArchive);
    connect(_ui.archiveListWidget, &ArchiveListWidget::displayJobDetails,
            _ui.jobListWidget, &JobListWidget::inspectJobByRef);
    connect(_ui.archiveDetailsWidget, &ArchiveWidget::jobClicked,
            _ui.jobListWidget, &JobListWidget::inspectJobByRef);
    connect(_ui.archiveDetailsWidget, &ArchiveWidget::restoreArchive, this,
            &MainWindow::restoreArchive);

    connect(_ui.archiveListWidget,
            &ArchiveListWidget::customContextMenuRequested, this,
            &MainWindow::showArchiveListMenu);
    connect(_ui.actionRefresh, &QAction::triggered, this,
            &MainWindow::getArchives);
    connect(_ui.actionDelete, &QAction::triggered, _ui.archiveListWidget,
            &ArchiveListWidget::deleteSelectedItems);
    connect(_ui.actionRestore, &QAction::triggered, _ui.archiveListWidget,
            &ArchiveListWidget::restoreSelectedItem);
    connect(_ui.actionInspect, &QAction::triggered, _ui.archiveListWidget,
            &ArchiveListWidget::inspectSelectedItem);

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
    connect(this, &MainWindow::jobsList, _ui.jobListWidget,
            &JobListWidget::setJobs);
    connect(_ui.jobListWidget, &JobListWidget::customContextMenuRequested,
            this, &MainWindow::showJobsListMenu);
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
    connect(_ui.downloadsDirLineEdit, &QLineEdit::textChanged, [&]() {
        QFileInfo file(_ui.downloadsDirLineEdit->text());
        if(file.exists() && file.isDir() && file.isWritable())
            _ui.downloadsDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
        else
            _ui.downloadsDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
    });
    connect(_ui.simulationCheckBox, &QCheckBox::stateChanged, [&](int state) {
        if(state == Qt::Unchecked)
        {
            emit getArchives();
            _ui.simulationIcon->hide();
        }
        else
        {
            _ui.simulationIcon->show();
        }
    });
    connect(_ui.repairCacheButton, &QPushButton::clicked, this,
            [&]() { emit repairCache(true); });
    connect(_ui.skipSystemDefaultsButton, &QPushButton::clicked,
            [&]() { _ui.skipSystemLineEdit->setText(DEFAULT_SKIP_SYSTEM_FILES); });
    connect(_ui.iecPrefixesCheckBox, &QCheckBox::toggled, this, [&]() {
        QMessageBox::information(this, QApplication::applicationName(),
                                 tr("The new size notation will take global "
                                    "effect on application restart."));
    });
    connect(_ui.dismissButton, &QPushButton::clicked, [&]() {
        QSettings settings;
        settings.setValue("app/default_jobs_dismissed", true);
        _ui.defaultJobs->hide();
        _ui.addJobButton->show();
    });
    connect(_ui.actionFilterArchives, &QAction::triggered, [&]()
    {
        _ui.archivesFilterFrame->setVisible(!_ui.archivesFilterFrame->isVisible());
        if(_ui.archivesFilter->isVisible())
            _ui.archivesFilter->setFocus();
        else
            _ui.archivesFilter->clearEditText();
    });
    connect(_ui.actionFilterJobs, &QAction::triggered, [&]()
    {
        _ui.jobsFilterFrame->setVisible(!_ui.jobsFilterFrame->isVisible());
        if(_ui.jobsFilter->isVisible())
            _ui.jobsFilter->setFocus();
        else
            _ui.jobsFilter->clearEditText();
    });
    connect(_ui.archivesFilter, &QComboBox::editTextChanged, _ui.archiveListWidget,
            &ArchiveListWidget::setFilter);
    connect(_ui.jobsFilter, &QComboBox::editTextChanged, _ui.jobListWidget,
            &JobListWidget::setFilter);
    connect(_ui.archivesFilter,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [&](){_ui.archiveListWidget->setFocus();});
    connect(_ui.jobsFilter,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [&](){_ui.jobListWidget->setFocus();});
    connect(_ui.languageComboBox, &QComboBox::currentTextChanged, this,
            [&](const QString language)
    {
        if(!language.isEmpty())
        {
            this->commitSettings();
            Translator &translator = Translator::instance();
            translator.translateApp(qApp, language);
        }
    });
    connect(_ui.archiveListWidget, &ArchiveListWidget::countChanged, this,
            [&](int total, int visible)
    {
        _ui.archivesCountLabel->setText(tr("Archives (%1/%2)")
                                        .arg(visible).arg(total));
    });
    connect(_ui.jobListWidget, &JobListWidget::countChanged, this,
            [&](int total, int visible)
    {
        _ui.jobsCountLabel->setText(tr("Jobs (%1/%2)")
                                    .arg(visible).arg(total));
    });
    connect(_ui.actionShowArchivesTabHeader, &QAction::triggered,
            [&](bool checked)
    {
        _ui.archivesHeader->setVisible(checked);
        QSettings settings;
        settings.setValue("app/archives_header_enabled", checked);
    });
    connect(_ui.actionShowJobsTabHeader, &QAction::triggered,
            [&](bool checked)
    {
        _ui.jobsHeader->setVisible(checked);
        QSettings settings;
        settings.setValue("app/jobs_header_enabled", checked);
    });
}

MainWindow::~MainWindow()
{
    commitSettings();
}

void MainWindow::loadSettings()
{
    QSettings settings;

    _ui.accountCreditLabel->setText(
        settings.value("tarsnap/credit", tr("click update button")).toString());
    _ui.machineActivity->setText(
        settings.value("tarsnap/machine_activity", tr("click update button")).toString());
    _ui.accountUserLineEdit->setText(
        settings.value("tarsnap/user", "").toString());
    _ui.accountMachineKeyLineEdit->setText(
        settings.value("tarsnap/key", "").toString());
    _ui.accountMachineLineEdit->setText(
        settings.value("tarsnap/machine", "").toString());
    _ui.tarsnapPathLineEdit->setText(
        settings.value("tarsnap/path", "").toString());
    _ui.tarsnapCacheLineEdit->setText(
        settings.value("tarsnap/cache", "").toString());
    _ui.aggressiveNetworkingCheckBox->setChecked(
        settings.value("tarsnap/aggressive_networking", DEFAULT_AGGRESSIVE_NETWORKING).toBool());
    _ui.traverseMountCheckBox->setChecked(
        settings.value("tarsnap/traverse_mount", DEFAULT_TRAVERSE_MOUNT).toBool());
    _ui.followSymLinksCheckBox->setChecked(
        settings.value("tarsnap/follow_symlinks", DEFAULT_FOLLOW_SYMLINKS).toBool());
    _ui.preservePathsCheckBox->setChecked(
        settings.value("tarsnap/preserve_pathnames", DEFAULT_PRESERVE_PATHNAMES).toBool());
    _ui.ignoreConfigCheckBox->setChecked(
        settings.value("tarsnap/no_default_config", DEFAULT_NO_DEFAULT_CONFIG).toBool());
    _ui.simulationCheckBox->setChecked(
        settings.value("tarsnap/dry_run", DEFAULT_DRY_RUN).toBool());
    _ui.simulationIcon->setVisible(_ui.simulationCheckBox->isChecked());
    _ui.iecPrefixesCheckBox->setChecked(
        settings.value("app/iec_prefixes", false).toBool());
    _ui.skipFilesSizeSpinBox->setValue(
        settings.value("app/skip_files_size", DEFAULT_SKIP_FILES_SIZE).toInt());
    _ui.skipSystemJunkCheckBox->setChecked(
        settings.value("app/skip_system_enabled", DEFAULT_SKIP_SYSTEM_ENABLED).toBool());
    _ui.skipSystemLineEdit->setEnabled(_ui.skipSystemJunkCheckBox->isChecked());
    _ui.skipSystemLineEdit->setText(
        settings.value("app/skip_system_files", DEFAULT_SKIP_SYSTEM_FILES).toString());
    _ui.skipNoDumpCheckBox->setChecked(
        settings.value("app/skip_nodump", DEFAULT_SKIP_NODUMP).toBool());
    _ui.downloadsDirLineEdit->setText(
        settings.value("app/downloads_dir", DEFAULT_DOWNLOADS).toString());
    _ui.appDataDirLineEdit->setText(
        settings.value("app/app_data", "").toString());
    _ui.notificationsCheckBox->setChecked(
        settings.value("app/notifications", true).toBool());
    _ui.limitUploadSpinBox->setValue(
        settings.value("app/limit_upload", 0).toInt());
    _ui.limitDownloadSpinBox->setValue(
                settings.value("app/limit_download", 0).toInt());
    _ui.actionShowArchivesTabHeader->setChecked(
                settings.value("app/archives_header_enabled", true).toBool());
    _ui.archivesHeader->setVisible(_ui.actionShowArchivesTabHeader->isChecked());
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

    Translator &translator = Translator::instance();
    _ui.languageComboBox->addItem(LANG_AUTO);
    _ui.languageComboBox->addItems(translator.languageList());
    _ui.languageComboBox->setCurrentText(settings.value("app/language",
                                                        LANG_AUTO)
                                         .toString());

    restoreGeometry(settings.value("app/window_geometry").toByteArray());
}

void MainWindow::initialize()
{
    QSettings settings;
    QDate creditDate = settings.value("tarsnap/credit_date", QDate()).toDate();
    if(creditDate.isValid())
    {
        _ui.accountCreditLabel->setToolTip(creditDate.toString());
        qint64 daysElapsed = creditDate.daysTo(QDate::currentDate());
        if(daysElapsed > 10)
        {
            _ui.outOfDateNoticeLabel->setText(
                _ui.outOfDateNoticeLabel->text().arg(daysElapsed));
            _ui.outOfDateNoticeLabel->show();
        }
        else
        {
            _ui.outOfDateNoticeLabel->hide();
        }
    }

    if(!validateTarsnapPath())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Tarsnap CLI utilities not found. Go to "
                                 " Settings -> Application page to fix that."));
    }

    if(!validateMachineKeyPath())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Machine key file not found. Go to "
                                 " Settings -> Account page to fix that."));
    }

    if(!validateTarsnapCache())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Tarsnap cache dir is invalid. Go to "
                                 " Settings -> Application page to fix that."));
    }

    if(!validateAppDataDir())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Application data dir is invalid. Go to "
                                 " Settings -> Application page to fix that."));
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(_ui.mainTabWidget->currentWidget() == _ui.archivesTab)
        {
            if(_ui.archiveDetailsWidget->isVisible())
            {
                _ui.archiveDetailsWidget->close();
                return;
            }
            if(_ui.archivesFilter->isVisible())
            {
                if(_ui.archivesFilter->currentText().isEmpty())
                {
                    _ui.actionFilterArchives->trigger();
                }
                else
                {
                    _ui.archivesFilter->clearEditText();
                    _ui.archivesFilter->setFocus();
                }
                return;
            }
        }
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
        emit getTaskInfo();
        event->ignore();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui.retranslateUi(this);
        updateUi();
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
    connect(actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    QAction *actionSettings = new QAction(this);
    actionSettings->setMenuRole(QAction::PreferencesRole);
    connect(actionSettings, &QAction::triggered, _ui.actionGoSettings, &QAction::trigger);
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
    connect(actionFullScreen, &QAction::triggered, [=](bool checked)
    {
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

    QMenu *helpMenu = _menuBar->addMenu(tr("&Help"));
    QAction *actionTarsnapWebsite = new QAction(tr("Tarsnap Website"), this);
    connect(actionTarsnapWebsite, &QAction::triggered, []()
    {
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

void MainWindow::overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
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
    _ui.accountTotalSizeLabel->setText(
        Utils::humanBytes(sizeTotal));
    _ui.accountTotalSizeLabel->setToolTip(tooltip);
    _ui.accountActualSizeLabel->setText(
        Utils::humanBytes(sizeUniqueCompressed));
    _ui.accountActualSizeLabel->setToolTip(tooltip);
    quint64 storageSaved =
        sizeTotal >= sizeUniqueCompressed ? sizeTotal - sizeUniqueCompressed : 0;
    _ui.accountStorageSavedLabel->setText(
        Utils::humanBytes(storageSaved));
    _ui.accountStorageSavedLabel->setToolTip(tooltip);
    _ui.accountArchivesCountLabel->setText(QString::number(archiveCount));
}

void MainWindow::updateTarsnapVersion(QString versionString)
{
    _ui.tarsnapVersionLabel->setText(versionString);
    QSettings settings;
    settings.setValue("tarsnap/version", versionString);
}

void MainWindow::createJobClicked()
{
    JobPtr job(new Job());
    job->setUrls(_ui.backupListWidget->itemUrls());
    job->setName(_ui.backupNameLineEdit->text());
    displayJobDetails(job);
    _ui.addJobButton->setEnabled(true);
    _ui.addJobButton->setText(tr("Save"));
    _ui.addJobButton->setProperty("save", true);
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
        _ui.backupDetailLabel->setText(tr("%1 %2 (%3)")
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
    if(archive->sizeTotal() == 0)
        emit loadArchiveStats(archive);

    if(archive->contents().count() == 0)
        emit loadArchiveContents(archive);

    _ui.archiveListWidget->selectArchive(archive);

    _ui.archiveDetailsWidget->setArchive(archive);
    if(!_ui.archiveDetailsWidget->isVisible())
        _ui.archiveDetailsWidget->show();

    if(_ui.mainTabWidget->currentWidget() != _ui.archivesTab)
        _ui.mainTabWidget->setCurrentWidget(_ui.archivesTab);
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
        urls << static_cast<BackupListWidgetItem *>(_ui.backupListWidget->item(i))->url();

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
    DEBUG << "COMMIT SETTINGS";
    QSettings settings;
    settings.setValue("tarsnap/path",    _ui.tarsnapPathLineEdit->text());
    settings.setValue("tarsnap/cache",   _ui.tarsnapCacheLineEdit->text());
    settings.setValue("tarsnap/key",     _ui.accountMachineKeyLineEdit->text());
    settings.setValue("tarsnap/machine", _ui.accountMachineLineEdit->text());
    settings.setValue("tarsnap/user",    _ui.accountUserLineEdit->text());
    settings.setValue("tarsnap/aggressive_networking", _ui.aggressiveNetworkingCheckBox->isChecked());
    settings.setValue("tarsnap/preserve_pathnames", _ui.preservePathsCheckBox->isChecked());
    settings.setValue("tarsnap/traverse_mount", _ui.traverseMountCheckBox->isChecked());
    settings.setValue("tarsnap/follow_symlinks", _ui.followSymLinksCheckBox->isChecked());
    settings.setValue("tarsnap/no_default_config", _ui.ignoreConfigCheckBox->isChecked());
    settings.setValue("tarsnap/dry_run", _ui.simulationCheckBox->isChecked());
    settings.setValue("app/iec_prefixes", _ui.iecPrefixesCheckBox->isChecked());
    settings.setValue("app/skip_files_size", _ui.skipFilesSizeSpinBox->value());
    settings.setValue("app/skip_system_enabled", _ui.skipSystemJunkCheckBox->isChecked());
    settings.setValue("app/skip_system_files", _ui.skipSystemLineEdit->text());
    settings.setValue("app/skip_nodump", _ui.skipNoDumpCheckBox->isChecked());
    settings.setValue("app/downloads_dir", _ui.downloadsDirLineEdit->text());
    settings.setValue("app/app_data", _ui.appDataDirLineEdit->text());
    settings.setValue("app/notifications", _ui.notificationsCheckBox->isChecked());
    settings.setValue("app/limit_upload", _ui.limitUploadSpinBox->value());
    settings.setValue("app/limit_download", _ui.limitDownloadSpinBox->value());
    settings.setValue("app/window_geometry", saveGeometry());
    settings.setValue("app/language", _ui.languageComboBox->currentText());
    settings.sync();
}

bool MainWindow::validateMachineKeyPath()
{
    QFileInfo machineKeyFile(_ui.accountMachineKeyLineEdit->text());
    if(machineKeyFile.exists() && machineKeyFile.isFile() &&
       machineKeyFile.isReadable())
    {
        _ui.accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: black;}");
        return true;
    }
    else
    {
        _ui.accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: red;}");
        return false;
    }
}

bool MainWindow::validateTarsnapPath()
{
    if(Utils::findTarsnapClientInPath(_ui.tarsnapPathLineEdit->text()).isEmpty())
    {
        _ui.tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: red;}");
        _ui.tarsnapVersionLabel->clear();
        return false;
    }
    else
    {
        _ui.tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: black;}");
        emit getTarsnapVersion(_ui.tarsnapPathLineEdit->text());
        return true;
    }
}

bool MainWindow::validateTarsnapCache()
{
    if(Utils::validateTarsnapCache(_ui.tarsnapCacheLineEdit->text()).isEmpty())
    {
        _ui.tarsnapCacheLineEdit->setStyleSheet("QLineEdit {color: red;}");
        return false;
    }
    else
    {
        _ui.tarsnapCacheLineEdit->setStyleSheet("QLineEdit {color: black;}");
        return true;
    }
}

bool MainWindow::validateAppDataDir()
{
    if(Utils::validateAppDataDir(_ui.appDataDirLineEdit->text()).isEmpty())
    {
        _ui.appDataDirLineEdit->setStyleSheet("QLineEdit {color: red;}");
        return false;
    }
    else
    {
        _ui.appDataDirLineEdit->setStyleSheet("QLineEdit {color: black;}");
        return true;
    }
}

void MainWindow::purgeTimerFired()
{
    if(_purgeTimerCount <= 1)
    {
        _purgeTimer.stop();
        _purgeCountdown.accept();
        emit purgeArchives();
    }
    else
    {
        --_purgeTimerCount;
        _purgeCountdown.setText(
            tr("Purging all archives in %1 seconds...").arg(_purgeTimerCount));
    }
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
    QTextCursor cursor(_ui.consoleLog->document());
    if(!_ui.consoleLog->document()->isEmpty())
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
    _ui.consoleLog->moveCursor(QTextCursor::End);
    _ui.consoleLog->ensureCursorVisible();
}

void MainWindow::setJournal(QVector<LogEntry> _log)
{
    _ui.journalLog->clear();
    foreach(LogEntry entry, _log)
        appendToJournalLog(entry);
}

void MainWindow::saveKeyId(QString key, int id)
{
    if(key == _ui.accountMachineKeyLineEdit->text())
    {
        QSettings settings;
        settings.setValue("tarsnap/key_id", id);
        settings.sync();
    }
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
                                    "Nothing to back up.").arg(job->name()));
            return;
        }
        else
        {
            auto confirm = QMessageBox::question(this, tr("Job warning"),
                                  tr("Some backup paths for Job %1 are not"
                                     " accessible anymore and thus backup may"
                                     " be incomplete."
                                     " Proceed with backup?").arg(job->name()));
            if(confirm != QMessageBox::Yes)
                return;
        }
    }
    emit backupNow(job->createBackupTask());
}

void MainWindow::browseForBackupItems()
{
    if(_ui.mainTabWidget->currentWidget() != _ui.backupTab)
        _ui.mainTabWidget->setCurrentWidget(_ui.backupTab);
    FilePickerDialog picker(this);
    connect(_ui.backupListWidget, &BackupListWidget::itemWithUrlAdded,
            &picker, &FilePickerDialog::selectUrl);
    picker.setSelectedUrls(_ui.backupListWidget->itemUrls());
    if(picker.exec())
        _ui.backupListWidget->setItemsWithUrls(picker.getSelectedUrls());
}

void MainWindow::accountMachineUseHostnameButtonClicked()
{
    _ui.accountMachineLineEdit->setText(QHostInfo::localHostName());
    commitSettings();
}

void MainWindow::accountMachineKeyBrowseButtonClicked()
{
    QString key =
        QFileDialog::getOpenFileName(this,
                                     tr("Browse for existing machine key"));
    if(!key.isEmpty())
    {
        _ui.accountMachineKeyLineEdit->setText(key);
        commitSettings();
    }
}

void MainWindow::tarsnapPathBrowseButtonClicked()
{
    QString tarsnapPath =
        QFileDialog::getExistingDirectory(this, tr("Find Tarsnap client"),
                                          _ui.tarsnapPathLineEdit->text());
    if(!tarsnapPath.isEmpty())
    {
        _ui.tarsnapPathLineEdit->setText(tarsnapPath);
        commitSettings();
    }
}

void MainWindow::tarsnapCacheBrowseButton()
{
    QString tarsnapCacheDir =
        QFileDialog::getExistingDirectory(this, tr("Tarsnap cache location"),
                                          _ui.tarsnapCacheLineEdit->text());
    if(!tarsnapCacheDir.isEmpty())
    {
        _ui.tarsnapCacheLineEdit->setText(tarsnapCacheDir);
        commitSettings();
    }
}

void MainWindow::appDataButtonClicked()
{
    QString appDataDir =
        QFileDialog::getExistingDirectory(this,
                                          tr("App data directory location"),
                                          _ui.appDataDirLineEdit->text());
    if(!appDataDir.isEmpty())
    {
        _ui.appDataDirLineEdit->setText(appDataDir);
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
            .arg(_ui.accountArchivesCountLabel->text(), confirmationText),
        QLineEdit::Normal, "", &ok);
    if(ok && (confirmationText == userText))
    {
        _purgeTimerCount = PURGE_SECONDS_DELAY;
        _purgeCountdown.setWindowTitle(
                            tr("Deleting all archives: press Cancel to abort"));
        _purgeCountdown.setText(
            tr("Purging all archives in %1 seconds...").arg(_purgeTimerCount));
        _purgeTimer.start(1000);
        if(QMessageBox::Cancel == _purgeCountdown.exec())
        {
            _purgeTimer.stop();
            updateStatusMessage(tr("Purge cancelled."));
        }
    }
}

void MainWindow::runSetupWizardClicked()
{
    if(_ui.busyWidget->isVisible())
    {
        QMessageBox::warning(this, tr("Confirm action"),
                              tr("Tasks are currently running. Please "
                                 "stop executing tasks or wait for "
                                 "completion and try again."));
        return;
    }
    auto confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Reset current app settings, job definitions "
                                 "and run the setup wizard?"));
    if(confirm == QMessageBox::Yes)
        emit runSetupWizard();
}

void MainWindow::downloadsDirBrowseButtonClicked()
{
    QString downDir =
        QFileDialog::getExistingDirectory(this,
                                          tr("Browse for downloads directory"),
                                          DEFAULT_DOWNLOADS);
    if(!downDir.isEmpty())
    {
        _ui.downloadsDirLineEdit->setText(downDir);
        commitSettings();
    }
}

void MainWindow::displayJobDetails(JobPtr job)
{
    if(!job)
        return;

    _ui.jobListWidget->selectJob(job);
    hideJobDetails();
    _ui.jobDetailsWidget->setJob(job);
    _ui.jobDetailsWidget->show();
    if(_ui.mainTabWidget->currentWidget() != _ui.jobsTab)
        _ui.mainTabWidget->setCurrentWidget(_ui.jobsTab);
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

void MainWindow::displayStopTasks(bool backupTaskRunning, int runningTasks,
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
            QMessageBox::information(this, tr("Stop Tasks"),
                                     tr("There are no running or queued tasks."));
            return;
        }
    }

    QMessageBox msgBox(this);
    msgBox.setText(tr("There are %1 running tasks and %2 queued.")
                       .arg(runningTasks)
                       .arg(queuedTasks));
    msgBox.setInformativeText(tr("What do you want to do?"));

    QPushButton actionButton(&msgBox);
    actionButton.setText(tr("Choose action"));
    QMenu actionMenu(&actionButton);
    QAction *interruptBackup = nullptr;
    if(backupTaskRunning)
    {
        if(_aboutToQuit)
            interruptBackup = actionMenu.addAction(tr("Interrupt backup and clear queue"));
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
    QPushButton *cancel = msgBox.addButton(QMessageBox::Cancel);
    msgBox.setDefaultButton(cancel);
    connect(&actionMenu, &QMenu::triggered, &msgBox, &QDialog::accept, Qt::QueuedConnection);
    actionButton.setMenu(&actionMenu);
    msgBox.addButton(&actionButton, QMessageBox::ActionRole);
    msgBox.exec();

    if((msgBox.clickedButton() == cancel) && _aboutToQuit)
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
                                 " Application."));
        break;
    }
    }
}

void MainWindow::updateAccountCredit(qreal credit, QDate date)
{
    QSettings settings;
    settings.setValue("tarsnap/credit", QString::number(credit, 'f', 18));
    settings.setValue("tarsnap/credit_date", date);
    _ui.accountCreditLabel->setText(QString::number(credit, 'f', 18));
    _ui.accountCreditLabel->setToolTip(date.toString());
    _ui.outOfDateNoticeLabel->hide();
}

void MainWindow::updateLastMachineActivity(QStringList activityFields)
{
    if(activityFields.size() < 2)
        return;
    QString machineActivity = activityFields[0] + ' ' + activityFields[1];
    QSettings settings;
    settings.setValue("tarsnap/machine_activity", machineActivity);
    _ui.machineActivity->setText(machineActivity);
    _ui.machineActivity->setToolTip(activityFields.join(' '));
    _ui.machineActivity->resize(_ui.machineActivity->fontMetrics()
                                 .width(_ui.machineActivity->text()),
                                 _ui.machineActivity->sizeHint().height());
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
    QPoint globalPos = _ui.archiveListWidget->viewport()->mapToGlobal(pos);
    QMenu  archiveListMenu(_ui.archiveListWidget);
    if(!_ui.archiveListWidget->selectedItems().isEmpty())
    {
        if(_ui.archiveListWidget->selectedItems().count() == 1)
        {
            archiveListMenu.addAction(_ui.actionInspect);
            archiveListMenu.addAction(_ui.actionRestore);
        }
        archiveListMenu.addAction(_ui.actionDelete);
    }
    archiveListMenu.addAction(_ui.actionRefresh);
    archiveListMenu.exec(globalPos);
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
    _ui.keyboardShortcuts->setPlainText(_ui.keyboardShortcuts->toPlainText()
                                        .arg(QKeySequence(Qt::ControlModifier)
                                             .toString(QKeySequence::NativeText))
                                        .arg(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier)
                                             .toString(QKeySequence::NativeText))
                                        .arg(QKeySequence(Qt::Key_Backspace)
                                             .toString(QKeySequence::NativeText))
                                        .arg(QKeySequence(Qt::Key_Delete)
                                             .toString(QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(0,
                                     _ui.mainTabWidget->tabToolTip(0)
                                     .arg(_ui.actionGoBackup->shortcut()
                                          .toString(QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(1,
                                     _ui.mainTabWidget->tabToolTip(1)
                                     .arg(_ui.actionGoArchives->shortcut()
                                          .toString(QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(2,
                                     _ui.mainTabWidget->tabToolTip(2)
                                     .arg(_ui.actionGoJobs->shortcut()
                                          .toString(QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(3,
                                     _ui.mainTabWidget->tabToolTip(3)
                                     .arg(_ui.actionGoSettings->shortcut()
                                          .toString(QKeySequence::NativeText)));
    _ui.mainTabWidget->setTabToolTip(4,
                                     _ui.mainTabWidget->tabToolTip(4)
                                     .arg(_ui.actionGoHelp->shortcut()
                                          .toString(QKeySequence::NativeText)));

    _ui.actionBackupNow->setToolTip(_ui.actionBackupNow->toolTip()
                                    .arg(_ui.actionBackupNow->shortcut()
                                         .toString(QKeySequence::NativeText)));
    _ui.backupListInfoLabel->setToolTip(_ui.backupListInfoLabel->toolTip()
                                        .arg(_ui.actionBrowseItems->shortcut()
                                         .toString(QKeySequence::NativeText)));
    _ui.actionShowJournal->setToolTip(_ui.actionShowJournal->toolTip()
                                      .arg(_ui.actionShowJournal->shortcut()
                                           .toString(QKeySequence::NativeText)));
    _ui.busyWidget->setToolTip(_ui.busyWidget->toolTip()
                               .arg(_ui.actionStopTasks->shortcut()
                                    .toString(QKeySequence::NativeText)));
    _ui.addJobButton->setToolTip(_ui.addJobButton->toolTip()
                                 .arg(_ui.actionAddJob->shortcut()
                                      .toString(QKeySequence::NativeText)));
    _ui.actionFilterArchives->setToolTip(_ui.actionFilterArchives->toolTip()
                                         .arg(_ui.actionFilterArchives->shortcut()
                                              .toString(QKeySequence::NativeText)));
    _ui.archivesFilter->setToolTip(_ui.archivesFilter->toolTip()
                                   .arg(_ui.actionFilterArchives->shortcut()
                                        .toString(QKeySequence::NativeText)));
    _ui.actionFilterJobs->setToolTip(_ui.actionFilterJobs->toolTip()
                                         .arg(_ui.actionFilterJobs->shortcut()
                                              .toString(QKeySequence::NativeText)));
    _ui.jobsFilter->setToolTip(_ui.jobsFilter->toolTip()
                                   .arg(_ui.actionFilterJobs->shortcut()
                                        .toString(QKeySequence::NativeText)));
    _ui.updateAccountButton->setToolTip(_ui.updateAccountButton->toolTip()
                                        .arg(_ui.actionRefreshAccount->shortcut()
                                             .toString(QKeySequence::NativeText)));
    // --

    setupMenuBar();

    if(_ui.addJobButton->property("save").toBool())
        _ui.addJobButton->setText(tr("Save"));
    else
        _ui.addJobButton->setText(tr("Add job"));
}

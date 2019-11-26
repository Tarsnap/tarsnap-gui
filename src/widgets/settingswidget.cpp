#include "settingswidget.h"

WARNINGS_DISABLE
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
#include <QWidget>

#include "ui_settingswidget.h"
WARNINGS_ENABLE

#include "translator.h"
#include "utils.h"

#include <ConsoleLog.h>
#include <TSettings.h>

#define NUKE_SECONDS_DELAY 8

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::SettingsWidget),
      _nukeConfirmationDialog(this),
      _tarsnapAccount(this)
{

    // Ui initialization
    _ui->setupUi(this);

    // Settings
    loadSettings();
    _ui->outOfDateNoticeLabel->hide();

    /* Account tab */

    // Settings
    connect(_ui->accountUserLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->accountMachineLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->accountMachineKeyLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->accountMachineKeyLineEdit, &QLineEdit::textChanged, this,
            &SettingsWidget::validateMachineKeyPath);
    connect(_ui->accountMachineUseHostnameButton, &QPushButton::clicked, this,
            &SettingsWidget::accountMachineUseHostnameButtonClicked);
    connect(_ui->accountMachineKeyBrowseButton, &QPushButton::clicked, this,
            &SettingsWidget::accountMachineKeyBrowseButtonClicked);
    connect(_ui->nukeArchivesButton, &QPushButton::clicked, this,
            &SettingsWidget::nukeArchivesButtonClicked);
    connect(&_nukeConfirmationDialog, &ConfirmationDialog::confirmed, this,
            &SettingsWidget::nukeConfirmed);
    connect(&_nukeConfirmationDialog, &ConfirmationDialog::cancelled, this,
            &SettingsWidget::nukeCancelled);
    connect(&_tarsnapAccount, &TarsnapAccountDialog::accountCredit, this,
            &SettingsWidget::updateAccountCredit);
    connect(&_tarsnapAccount, &TarsnapAccountDialog::getKeyId, this,
            &SettingsWidget::getKeyId);
    connect(_ui->updateAccountButton, &QPushButton::clicked,
            _ui->actionRefreshAccount, &QAction::trigger);
    connect(&_tarsnapAccount, &TarsnapAccountDialog::lastMachineActivity, this,
            &SettingsWidget::updateLastMachineActivity);
    connect(_ui->accountActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(true, false); });
    connect(_ui->machineActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(false, true); });
    connect(_ui->actionRefreshAccount, &QAction::triggered, this,
            &SettingsWidget::getAccountInfo);

    /* Backup tab */
    connect(_ui->aggressiveNetworkingCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->preservePathsCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->traverseMountCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->followSymLinksCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->skipFilesSizeSpinBox, &QSpinBox::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->skipSystemJunkCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->skipSystemLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->skipNoDumpCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->simulationCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->ignoreConfigCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);

    connect(_ui->limitUploadSpinBox, &QSpinBox::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->limitDownloadSpinBox, &QSpinBox::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->enableSchedulingButton, &QPushButton::clicked, this,
            &SettingsWidget::enableJobSchedulingButtonClicked);
    connect(_ui->disableSchedulingButton, &QPushButton::clicked, this,
            &SettingsWidget::disableJobSchedulingButtonClicked);
    connect(_ui->simulationCheckBox, &QCheckBox::stateChanged, this,
            &SettingsWidget::updateSimulationIcon);
    connect(_ui->skipSystemDefaultsButton, &QPushButton::clicked, [&]() {
        _ui->skipSystemLineEdit->setText(DEFAULT_SKIP_SYSTEM_FILES);
    });

    /* Application tab */
    connect(_ui->tarsnapPathLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->tarsnapCacheLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->tarsnapPathLineEdit, &QLineEdit::textChanged, this,
            &SettingsWidget::validateTarsnapPath);
    connect(_ui->tarsnapCacheLineEdit, &QLineEdit::textChanged, this,
            &SettingsWidget::validateTarsnapCache);
    connect(_ui->appDataDirLineEdit, &QLineEdit::textChanged, this,
            &SettingsWidget::validateAppDataDir);
    connect(_ui->iecPrefixesCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->notificationsCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);
    connect(_ui->downloadsDirLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui->saveConsoleLogCheckBox, &QCheckBox::toggled, this,
            &SettingsWidget::commitSettings);

    connect(_ui->tarsnapPathBrowseButton, &QPushButton::clicked, this,
            &SettingsWidget::tarsnapPathBrowseButtonClicked);
    connect(_ui->tarsnapCacheBrowseButton, &QPushButton::clicked, this,
            &SettingsWidget::tarsnapCacheBrowseButton);
    connect(_ui->appDataDirBrowseButton, &QPushButton::clicked, this,
            &SettingsWidget::appDataButtonClicked);
    connect(_ui->runSetupWizard, &QPushButton::clicked, this,
            &SettingsWidget::runSetupWizardClicked);
    connect(_ui->downloadsDirBrowseButton, &QPushButton::clicked, this,
            &SettingsWidget::downloadsDirBrowseButtonClicked);
    connect(_ui->clearJournalButton, &QPushButton::clicked, this,
            &SettingsWidget::clearJournalClicked);

    connect(_ui->downloadsDirLineEdit, &QLineEdit::textChanged, [&]() {
        QFileInfo file(_ui->downloadsDirLineEdit->text());
        if(file.exists() && file.isDir() && file.isWritable())
            _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:black;}");
        else
            _ui->downloadsDirLineEdit->setStyleSheet("QLineEdit{color:red;}");
    });
    connect(_ui->repairCacheButton, &QPushButton::clicked, this,
            [&]() { emit repairCache(true); });
    connect(_ui->iecPrefixesCheckBox, &QCheckBox::toggled, this, [&]() {
        QMessageBox::information(this, QApplication::applicationName(),
                                 tr("The new size notation will take global "
                                    "effect on application restart."));
    });

    connect(_ui->languageComboBox, &QComboBox::currentTextChanged, this,
            [&](const QString language) {
                if(!language.isEmpty())
                {
                    this->commitSettings();
                    Translator &translator = Translator::instance();
                    translator.translateApp(qApp, language);
                }
            });

    updateUi();
}

SettingsWidget::~SettingsWidget()
{
    delete _ui;
}

void SettingsWidget::initializeSettingsWidget()
{
    TSettings settings;

    /* Account tab */

    // Check if we should show a "credit might be out of date" warning.
    QDate creditDate = settings.value("tarsnap/credit_date", QDate()).toDate();
    if(creditDate.isValid())
    {
        _ui->accountCreditLabel->setToolTip(creditDate.toString());
        qint64 daysElapsed = creditDate.daysTo(QDate::currentDate());
        if(daysElapsed > 0)
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

    if(!validateMachineKeyPath())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Machine key file not found. Go to "
                                 " Settings -> Account page to fix that."));
    }

    /* Application tab */

    // Validate applications paths.
    if(!validateTarsnapPath())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Tarsnap CLI utilities not found. Go to "
                                 " Settings -> Application page to fix that."));
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
}

void SettingsWidget::loadSettings()
{
    TSettings settings;

    /* Account tab */
    _ui->accountCreditLabel->setText(
        settings.value("tarsnap/credit", tr("click login button")).toString());
    _ui->machineActivity->setText(
        settings.value("tarsnap/machine_activity", tr("click login button"))
            .toString());
    _ui->accountUserLineEdit->setText(
        settings.value("tarsnap/user", "").toString());
    _ui->accountMachineKeyLineEdit->setText(
        settings.value("tarsnap/key", "").toString());
    _ui->accountMachineLineEdit->setText(
        settings.value("tarsnap/machine", "").toString());

    /* Backup tab */
    _ui->aggressiveNetworkingCheckBox->setChecked(
        settings
            .value("tarsnap/aggressive_networking",
                   DEFAULT_AGGRESSIVE_NETWORKING)
            .toBool());
    _ui->traverseMountCheckBox->setChecked(
        settings.value("tarsnap/traverse_mount", DEFAULT_TRAVERSE_MOUNT)
            .toBool());
    _ui->followSymLinksCheckBox->setChecked(
        settings.value("tarsnap/follow_symlinks", DEFAULT_FOLLOW_SYMLINKS)
            .toBool());
    _ui->preservePathsCheckBox->setChecked(
        settings.value("tarsnap/preserve_pathnames", DEFAULT_PRESERVE_PATHNAMES)
            .toBool());
    _ui->ignoreConfigCheckBox->setChecked(
        settings.value("tarsnap/no_default_config", DEFAULT_NO_DEFAULT_CONFIG)
            .toBool());
    _ui->simulationCheckBox->setChecked(
        settings.value("tarsnap/dry_run", DEFAULT_DRY_RUN).toBool());
    _ui->skipFilesSizeSpinBox->setValue(
        settings.value("app/skip_files_size", DEFAULT_SKIP_FILES_SIZE).toInt());
    _ui->skipSystemJunkCheckBox->setChecked(
        settings.value("app/skip_system_enabled", DEFAULT_SKIP_SYSTEM_ENABLED)
            .toBool());
    _ui->skipSystemLineEdit->setEnabled(
        _ui->skipSystemJunkCheckBox->isChecked());
    _ui->skipSystemLineEdit->setText(
        settings.value("app/skip_system_files", DEFAULT_SKIP_SYSTEM_FILES)
            .toString());
    _ui->skipNoDumpCheckBox->setChecked(
        settings.value("app/skip_nodump", DEFAULT_SKIP_NODUMP).toBool());
    _ui->limitUploadSpinBox->setValue(
        settings.value("app/limit_upload", 0).toInt());
    _ui->limitDownloadSpinBox->setValue(
        settings.value("app/limit_download", 0).toInt());

    /* Application tab */
    _ui->tarsnapPathLineEdit->setText(
        settings.value("tarsnap/path", "").toString());
    _ui->tarsnapCacheLineEdit->setText(
        settings.value("tarsnap/cache", "").toString());
    _ui->iecPrefixesCheckBox->setChecked(
        settings.value("app/iec_prefixes", false).toBool());
    _ui->downloadsDirLineEdit->setText(
        settings.value("app/downloads_dir", DEFAULT_DOWNLOADS).toString());
    _ui->appDataDirLineEdit->setText(
        settings.value("app/app_data", "").toString());
    _ui->notificationsCheckBox->setChecked(
        settings.value("app/notifications", true).toBool());
    _ui->saveConsoleLogCheckBox->setChecked(
        settings.value("app/save_console_log", false).toBool());
    _ui->saveConsoleLogLineEdit->setText(LOG.getLogFile());
    LOG.setWriteToFile(settings.value("app/save_console_log", false).toBool());

    Translator &translator = Translator::instance();
    _ui->languageComboBox->addItem(LANG_AUTO);
    _ui->languageComboBox->addItems(translator.languageList());
    _ui->languageComboBox->setCurrentText(
        settings.value("app/language", LANG_AUTO).toString());
}

void SettingsWidget::commitSettings()
{
    TSettings settings;

    /* Account tab */
    settings.setValue("tarsnap/key", _ui->accountMachineKeyLineEdit->text());
    settings.setValue("tarsnap/machine", _ui->accountMachineLineEdit->text());
    settings.setValue("tarsnap/user", _ui->accountUserLineEdit->text());

    /* Backup tab */
    settings.setValue("tarsnap/aggressive_networking",
                      _ui->aggressiveNetworkingCheckBox->isChecked());
    settings.setValue("tarsnap/preserve_pathnames",
                      _ui->preservePathsCheckBox->isChecked());
    settings.setValue("tarsnap/traverse_mount",
                      _ui->traverseMountCheckBox->isChecked());
    settings.setValue("tarsnap/follow_symlinks",
                      _ui->followSymLinksCheckBox->isChecked());
    settings.setValue("tarsnap/no_default_config",
                      _ui->ignoreConfigCheckBox->isChecked());
    settings.setValue("tarsnap/dry_run", _ui->simulationCheckBox->isChecked());
    settings.setValue("app/skip_files_size",
                      _ui->skipFilesSizeSpinBox->value());
    settings.setValue("app/skip_system_enabled",
                      _ui->skipSystemJunkCheckBox->isChecked());
    settings.setValue("app/skip_system_files", _ui->skipSystemLineEdit->text());
    settings.setValue("app/skip_nodump", _ui->skipNoDumpCheckBox->isChecked());

    settings.setValue("app/limit_upload", _ui->limitUploadSpinBox->value());
    settings.setValue("app/limit_download", _ui->limitDownloadSpinBox->value());

    /* Application tab */
    settings.setValue("tarsnap/path", _ui->tarsnapPathLineEdit->text());
    settings.setValue("tarsnap/cache", _ui->tarsnapCacheLineEdit->text());
    settings.setValue("app/iec_prefixes",
                      _ui->iecPrefixesCheckBox->isChecked());
    settings.setValue("app/downloads_dir", _ui->downloadsDirLineEdit->text());
    settings.setValue("app/app_data", _ui->appDataDirLineEdit->text());
    settings.setValue("app/notifications",
                      _ui->notificationsCheckBox->isChecked());
    settings.setValue("app/language", _ui->languageComboBox->currentText());
    settings.setValue("app/save_console_log",
                      _ui->saveConsoleLogCheckBox->isChecked());
    LOG.setWriteToFile(settings.value("app/save_console_log", false).toBool());

    settings.sync();
}

void SettingsWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void SettingsWidget::getAccountInfo()
{
    _tarsnapAccount.getAccountInfo();
}

void SettingsWidget::overallStatsChanged(quint64 sizeTotal,
                                         quint64 sizeCompressed,
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
    _ui->accountTotalSizeLabel->setText(Utils::humanBytes(sizeTotal));
    _ui->accountTotalSizeLabel->setToolTip(tooltip);
    _ui->accountActualSizeLabel->setText(
        Utils::humanBytes(sizeUniqueCompressed));
    _ui->accountActualSizeLabel->setToolTip(tooltip);
    quint64 storageSaved = sizeTotal >= sizeUniqueCompressed
                               ? sizeTotal - sizeUniqueCompressed
                               : 0;
    _ui->accountStorageSavedLabel->setText(Utils::humanBytes(storageSaved));
    _ui->accountStorageSavedLabel->setToolTip(tooltip);
    _ui->accountArchivesCountLabel->setText(QString::number(archiveCount));
}

bool SettingsWidget::validateMachineKeyPath()
{
    QFileInfo machineKeyFile(_ui->accountMachineKeyLineEdit->text());
    if(machineKeyFile.exists() && machineKeyFile.isFile()
       && machineKeyFile.isReadable())
    {
        _ui->accountMachineKeyLineEdit->setStyleSheet(
            "QLineEdit {color: black;}");
        return true;
    }
    else
    {
        _ui->accountMachineKeyLineEdit->setStyleSheet(
            "QLineEdit {color: red;}");
        return false;
    }
}

void SettingsWidget::saveKeyId(QString key, quint64 id)
{
    if(key == _ui->accountMachineKeyLineEdit->text())
    {
        TSettings settings;
        settings.setValue("tarsnap/key_id", id);
        settings.sync();
    }
}

void SettingsWidget::accountMachineUseHostnameButtonClicked()
{
    _ui->accountMachineLineEdit->setText(QHostInfo::localHostName());
    commitSettings();
}

void SettingsWidget::accountMachineKeyBrowseButtonClicked()
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

void SettingsWidget::nukeArchivesButtonClicked()
{
    const QString confirmationText = tr("No Tomorrow");

    newStatusMessage(tr("Nuke confirmation requested."), "");

    // Set up nuke confirmation
    _nukeConfirmationDialog.start(
        tr("Nuke all archives?"),
        tr("This action will <b>delete all (%1) archives</b> stored for this "
           "key."
           "<br /><br />To confirm, type '%2' and press OK."
           "<br /><br /><i>Warning: This action cannot be undone. "
           "All archives will be <b>lost forever</b></i>.")
            .arg(_ui->accountArchivesCountLabel->text(), confirmationText),
        confirmationText, NUKE_SECONDS_DELAY,
        tr("Deleting all archives: press Cancel to abort"),
        tr("Purging all archives in %1 seconds..."));
}

void SettingsWidget::nukeConfirmed()
{
    emit nukeArchives();
}

void SettingsWidget::nukeCancelled()
{
    newStatusMessage(tr("Nuke cancelled."), "");
}

void SettingsWidget::updateAccountCredit(qreal credit, QDate date)
{
    TSettings settings;
    settings.setValue("tarsnap/credit", QString::number(credit, 'f', 18));
    settings.setValue("tarsnap/credit_date", date);
    _ui->accountCreditLabel->setText(QString::number(credit, 'f', 18));
    _ui->accountCreditLabel->setToolTip(date.toString());
    _ui->outOfDateNoticeLabel->hide();
}

void SettingsWidget::updateLastMachineActivity(QStringList activityFields)
{
    if(activityFields.size() < 2)
        return;
    QString   machineActivity = activityFields[0] + ' ' + activityFields[1];
    TSettings settings;
    settings.setValue("tarsnap/machine_activity", machineActivity);
    _ui->machineActivity->setText(machineActivity);
    _ui->machineActivity->setToolTip(activityFields.join(' '));
    _ui->machineActivity->resize(_ui->machineActivity->fontMetrics()
                                     .boundingRect(_ui->machineActivity->text())
                                     .width(),
                                 _ui->machineActivity->sizeHint().height());
}

void SettingsWidget::updateUi()
{
    _ui->updateAccountButton->setToolTip(
        _ui->updateAccountButton->toolTip().arg(
            _ui->actionRefreshAccount->shortcut().toString(
                QKeySequence::NativeText)));
}

void SettingsWidget::updateSimulationIcon(int state)
{
    emit newSimulationStatus(state);
}

void SettingsWidget::enableJobSchedulingButtonClicked()
{
#if defined(Q_OS_OSX)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Job scheduling"),
                              tr("Register Tarsnap GUI with the OS X"
                                 " Launchd service to run daily at 10am?"
                                 "\n\nJobs that have scheduled backup"
                                 " turned on will be backed up according"
                                 " to the Daily, Weekly or Monthly"
                                 " schedule. \n\n%1")
                                  .arg(CRON_MARKER_HELP));
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = launchdEnable();
    if(info.status != SCHEDULE_OK)
    {
        QMessageBox::critical(this, tr("Job scheduling"), info.message);
        return;
    }

#elif defined(Q_OS_LINUX) || defined(Q_OS_BSD4)

    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Job scheduling"),
                              tr("Register Tarsnap GUI with cron serivce?"
                                 "\nJobs that have scheduled backup"
                                 " turned on will be backed up according"
                                 " to the Daily, Weekly or Monthly"
                                 " schedule. \n\n%1")
                                  .arg(CRON_MARKER_HELP));
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = cronEnable();
    if(info.status == SCHEDULE_ERROR)
    {
        QMessageBox::critical(this, tr("Job scheduling"), info.message);
        return;
    }
    else if(info.status == SCHEDULE_OK)
    {
        QMessageBox::critical(this, tr("Job scheduling"),
                              "Unknown error in scheduling code.");
        return;
    }
    QString cronBlock = info.message;

    QMessageBox question(this);
    question.setIcon(QMessageBox::Question);
    question.setText(QObject::tr(
        "Tarsnap GUI will be added to the current user's crontab."));
    question.setInformativeText(
        QObject::tr("To ensure proper behavior please review the"
                    " lines to be added by pressing Show"
                    " Details before proceeding."));
    question.setDetailedText(cronBlock);
    question.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    question.setDefaultButton(QMessageBox::Cancel);
    // Workaround for activating Show details by default
    for(QAbstractButton *button : question.buttons())
    {
        if(question.buttonRole(button) == QMessageBox::ActionRole)
        {
            button->click();
            break;
        }
    }
    int proceed = question.exec();
    if(proceed == QMessageBox::Cancel)
        return;

    struct scheduleinfo info_p2 = cronEnable_p2(cronBlock, info.extra);
    if(info_p2.status != SCHEDULE_OK)
    {
        QMessageBox::critical(this, tr("Job scheduling"), info.message);
        return;
    }
#endif
}

void SettingsWidget::disableJobSchedulingButtonClicked()
{
#if defined(Q_OS_OSX)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Job scheduling"),
                              tr("Unregister Tarsnap GUI from the OS X"
                                 " Launchd service? This will disable"
                                 " automatic Job backup scheduling."
                                 "\n\n%1")
                                  .arg(CRON_MARKER_HELP));
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = launchdDisable();
    if(info.status != SCHEDULE_OK)
    {
        QMessageBox::critical(this, tr("Job scheduling"), info.message);
        return;
    }

#elif defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, "Confirm action",
                              "Unregister Tarsnap GUI from cron?");
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = cronDisable();
    if(info.status == SCHEDULE_ERROR)
    {
        QMessageBox::critical(this, tr("Job scheduling"), info.message);
        return;
    }
    else if(info.status == SCHEDULE_OK)
    {
        QMessageBox::critical(this, tr("Job scheduling"),
                              "Unknown error in scheduling code.");
        return;
    }
    QString linesToRemove = info.message;

    QMessageBox question(this);
    question.setIcon(QMessageBox::Question);
    question.setText(tr("Tarsnap GUI will be removed from the current user's"
                        " crontab."));
    question.setInformativeText(
        tr("To ensure proper behavior please review the"
           " lines to be removed by pressing Show Details"
           " before proceeding."));
    question.setDetailedText(linesToRemove);
    question.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    question.setDefaultButton(QMessageBox::Cancel);
    // Workaround for activating Show details by default
    for(QAbstractButton *button : question.buttons())
    {
        if(question.buttonRole(button) == QMessageBox::ActionRole)
        {
            button->click();
            break;
        }
    }
    int proceed = question.exec();
    if(proceed == QMessageBox::Cancel)
        return;

    struct scheduleinfo info_p2 = cronDisable_p2(linesToRemove, info.extra);
    if(info_p2.status != SCHEDULE_OK)
    {
        QMessageBox::critical(this, tr("Job scheduling"), info.message);
        return;
    }
#endif
}

void SettingsWidget::tarsnapVersionResponse(QString versionString)
{
    _ui->tarsnapVersionLabel->setText(versionString);
    TSettings settings;
    settings.setValue("tarsnap/version", versionString);
}

bool SettingsWidget::validateTarsnapPath()
{
    if(Utils::findTarsnapClientInPath(_ui->tarsnapPathLineEdit->text())
           .isEmpty())
    {
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: red;}");
        _ui->tarsnapVersionLabel->clear();
        return false;
    }
    else
    {
        _ui->tarsnapPathLineEdit->setStyleSheet("QLineEdit {color: black;}");
        emit tarsnapVersionRequested(_ui->tarsnapPathLineEdit->text());
        return true;
    }
}

bool SettingsWidget::validateTarsnapCache()
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

bool SettingsWidget::validateAppDataDir()
{
    if(Utils::validateAppDataDir(_ui->appDataDirLineEdit->text()).isEmpty())
    {
        _ui->appDataDirLineEdit->setStyleSheet("QLineEdit {color: red;}");
        return false;
    }
    else
    {
        _ui->appDataDirLineEdit->setStyleSheet("QLineEdit {color: black;}");
        return true;
    }
}

void SettingsWidget::tarsnapPathBrowseButtonClicked()
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

void SettingsWidget::tarsnapCacheBrowseButton()
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

void SettingsWidget::appDataButtonClicked()
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

void SettingsWidget::runSetupWizardClicked()
{
    if((_runningTasks + _queuedTasks) > 0)
    {
        QMessageBox::warning(this, tr("Confirm action"),
                             tr("Tasks are currently running. Please "
                                "stop executing tasks or wait for "
                                "completion and try again."));
        return;
    }
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Reset current app settings, job definitions "
                                 "and run the setup wizard?"),
                              (QMessageBox::Yes | QMessageBox::No),
                              QMessageBox::No);
    if(confirm == QMessageBox::Yes)
        emit runSetupWizard();
}

void SettingsWidget::downloadsDirBrowseButtonClicked()
{
    QString downDir =
        QFileDialog::getExistingDirectory(this,
                                          tr("Browse for downloads directory"),
                                          DEFAULT_DOWNLOADS);
    if(!downDir.isEmpty())
    {
        _ui->downloadsDirLineEdit->setText(downDir);
        commitSettings();
    }
}

void SettingsWidget::clearJournalClicked()
{
    QMessageBox::StandardButton confirm =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Clear journal log? All entries will "
                                 "be deleted forever."));
    if(confirm == QMessageBox::Yes)
        emit clearJournal();
}

void SettingsWidget::updateNumTasks(int runningTasks, int queuedTasks)
{
    _runningTasks = runningTasks;
    _queuedTasks  = queuedTasks;
}

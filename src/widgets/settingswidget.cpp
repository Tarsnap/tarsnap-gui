#include "settingswidget.h"

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
#include <QWidget>

#define NUKE_SECONDS_DELAY 8

SettingsWidget::SettingsWidget(QWidget *parent)
    : QWidget(parent),
      _nukeTimerCount(0),
      _nukeCountdown(this),
      _tarsnapAccount(this),
      _nukeInput(this)
{

    // Ui initialization
    _ui.setupUi(this);

    // Settings
    loadSettings();
    _ui.outOfDateNoticeLabel->hide();

    /* Account tab */

    // Nuke widget setup
    _nukeCountdown.setIcon(QMessageBox::Critical);
    _nukeCountdown.setStandardButtons(QMessageBox::Cancel);
    connect(&_nukeTimer, &QTimer::timeout, this,
            &SettingsWidget::nukeTimerFired);

    // Settings
    connect(_ui.accountUserLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui.accountMachineLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui.accountMachineKeyLineEdit, &QLineEdit::editingFinished, this,
            &SettingsWidget::commitSettings);
    connect(_ui.accountMachineKeyLineEdit, &QLineEdit::textChanged, this,
            &SettingsWidget::validateMachineKeyPath);
    connect(_ui.accountMachineUseHostnameButton, &QPushButton::clicked, this,
            &SettingsWidget::accountMachineUseHostnameButtonClicked);
    connect(_ui.accountMachineKeyBrowseButton, &QPushButton::clicked, this,
            &SettingsWidget::accountMachineKeyBrowseButtonClicked);
    connect(_ui.nukeArchivesButton, &QPushButton::clicked, this,
            &SettingsWidget::nukeArchivesButtonClicked);
    connect(&_tarsnapAccount, &TarsnapAccount::accountCredit, this,
            &SettingsWidget::updateAccountCredit);
    connect(&_tarsnapAccount, &TarsnapAccount::getKeyId, this,
            &SettingsWidget::getKeyId);
    connect(_ui.updateAccountButton, &QPushButton::clicked,
            _ui.actionRefreshAccount, &QAction::trigger);
    connect(&_tarsnapAccount, &TarsnapAccount::lastMachineActivity, this,
            &SettingsWidget::updateLastMachineActivity);
    connect(_ui.accountActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(true, false); });
    connect(_ui.machineActivityShowButton, &QPushButton::clicked,
            [&]() { _tarsnapAccount.getAccountInfo(false, true); });
    connect(_ui.actionRefreshAccount, &QAction::triggered, this,
            &SettingsWidget::getAccountInfo);
}

void SettingsWidget::initializeSettingsWidget()
{
    QSettings settings;

    /* Account tab */

    // Check if we should show a "credit might be out of date" warning.
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

    if(!validateMachineKeyPath())
    {
        QMessageBox::critical(this, tr("Tarsnap error"),
                              tr("Machine key file not found. Go to "
                                 " Settings -> Account page to fix that."));
    }
}

void SettingsWidget::loadSettings()
{
    QSettings settings;

    /* Account tab */
    _ui.accountCreditLabel->setText(
        settings.value("tarsnap/credit", tr("click login button")).toString());
    _ui.machineActivity->setText(
        settings.value("tarsnap/machine_activity", tr("click login button"))
            .toString());
    _ui.accountUserLineEdit->setText(
        settings.value("tarsnap/user", "").toString());
    _ui.accountMachineKeyLineEdit->setText(
        settings.value("tarsnap/key", "").toString());
    _ui.accountMachineLineEdit->setText(
        settings.value("tarsnap/machine", "").toString());
}

void SettingsWidget::commitSettings()
{
    QSettings settings;

    /* Account tab */
    settings.setValue("tarsnap/key", _ui.accountMachineKeyLineEdit->text());
    settings.setValue("tarsnap/machine", _ui.accountMachineLineEdit->text());
    settings.setValue("tarsnap/user", _ui.accountUserLineEdit->text());

    settings.sync();
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
    _ui.accountTotalSizeLabel->setText(Utils::humanBytes(sizeTotal));
    _ui.accountTotalSizeLabel->setToolTip(tooltip);
    _ui.accountActualSizeLabel->setText(Utils::humanBytes(sizeUniqueCompressed));
    _ui.accountActualSizeLabel->setToolTip(tooltip);
    quint64 storageSaved =
        sizeTotal >= sizeUniqueCompressed ? sizeTotal - sizeUniqueCompressed : 0;
    _ui.accountStorageSavedLabel->setText(Utils::humanBytes(storageSaved));
    _ui.accountStorageSavedLabel->setToolTip(tooltip);
    _ui.accountArchivesCountLabel->setText(QString::number(archiveCount));
}

bool SettingsWidget::validateMachineKeyPath()
{
    QFileInfo machineKeyFile(_ui.accountMachineKeyLineEdit->text());
    if(machineKeyFile.exists() && machineKeyFile.isFile()
       && machineKeyFile.isReadable())
    {
        _ui.accountMachineKeyLineEdit->setStyleSheet(
            "QLineEdit {color: black;}");
        return true;
    }
    else
    {
        _ui.accountMachineKeyLineEdit->setStyleSheet("QLineEdit {color: red;}");
        return false;
    }
}

void SettingsWidget::saveKeyId(QString key, quint64 id)
{
    if(key == _ui.accountMachineKeyLineEdit->text())
    {
        QSettings settings;
        settings.setValue("tarsnap/key_id", id);
        settings.sync();
    }
}

void SettingsWidget::accountMachineUseHostnameButtonClicked()
{
    _ui.accountMachineLineEdit->setText(QHostInfo::localHostName());
    commitSettings();
}

void SettingsWidget::accountMachineKeyBrowseButtonClicked()
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

void SettingsWidget::nukeTimerFired()
{
    if(_nukeTimerCount <= 1)
    {
        _nukeTimer.stop();
        _nukeCountdown.accept();
        emit nukeArchives();
    }
    else
    {
        --_nukeTimerCount;
        _nukeCountdown.setText(
            tr("Purging all archives in %1 seconds...").arg(_nukeTimerCount));
    }
}

void SettingsWidget::nukeArchivesButtonClicked()
{
    const QString confirmationText = tr("No Tomorrow");

    // Set up nuke confirmation
    _nukeInput.setWindowTitle(tr("Nuke all archives?"));
    _nukeInput.setLabelText(
        tr("This action will <b>delete all (%1) archives</b> stored for this "
           "key."
           "<br /><br />To confirm, type '%2' and press OK."
           "<br /><br /><i>Warning: This action cannot be undone. "
           "All archives will be <b>lost forever</b></i>.")
            .arg(_ui.accountArchivesCountLabel->text(), confirmationText));
    _nukeInput.setInputMode(QInputDialog::TextInput);

    // Run nuke confirmation
    bool ok = _nukeInput.exec();

    if(ok && (confirmationText == _nukeInput.textValue()))
    {
        _nukeTimerCount = NUKE_SECONDS_DELAY;
        _nukeCountdown.setWindowTitle(
            tr("Deleting all archives: press Cancel to abort"));
        _nukeCountdown.setText(
            tr("Purging all archives in %1 seconds...").arg(_nukeTimerCount));
        _nukeTimer.start(1000);
        if(QMessageBox::Cancel == _nukeCountdown.exec())
        {
            _nukeTimer.stop();
            newStatusMessage(tr("Nuke cancelled."), "");
        }
    }
    else
    {
        newStatusMessage(tr("Nuke cancelled."), "");
    }
}

void SettingsWidget::updateAccountCredit(qreal credit, QDate date)
{
    QSettings settings;
    settings.setValue("tarsnap/credit", QString::number(credit, 'f', 18));
    settings.setValue("tarsnap/credit_date", date);
    _ui.accountCreditLabel->setText(QString::number(credit, 'f', 18));
    _ui.accountCreditLabel->setToolTip(date.toString());
    _ui.outOfDateNoticeLabel->hide();
}

void SettingsWidget::updateLastMachineActivity(QStringList activityFields)
{
    if(activityFields.size() < 2)
        return;
    QString   machineActivity = activityFields[0] + ' ' + activityFields[1];
    QSettings settings;
    settings.setValue("tarsnap/machine_activity", machineActivity);
    _ui.machineActivity->setText(machineActivity);
    _ui.machineActivity->setToolTip(activityFields.join(' '));
    _ui.machineActivity->resize(_ui.machineActivity->fontMetrics().width(
                                    _ui.machineActivity->text()),
                                _ui.machineActivity->sizeHint().height());
}

void SettingsWidget::updateUi()
{
    _ui.updateAccountButton->setToolTip(_ui.updateAccountButton->toolTip().arg(
        _ui.actionRefreshAccount->shortcut().toString(QKeySequence::NativeText)));
}

#include "backuptabwidget.h"

#include "persistentmodel/archive.h"
#include "utils.h"

#include <QDateTime>
#include <QSettings>
#include <QWidget>

BackupTabWidget::BackupTabWidget(QWidget *parent) : QWidget(parent)
{
    // Ui initialization
    _ui.setupUi(this);

    // Messages between widgets on this tab
    connect(_ui.appendTimestampCheckBox, &QCheckBox::toggled, this,
            &BackupTabWidget::appendTimestampCheckBoxToggled);
    connect(_ui.backupListInfoLabel, &ElidedLabel::clicked,
            _ui.actionBrowseItems, &QAction::trigger);
    connect(_ui.backupNameLineEdit, &QLineEdit::textChanged,
            [&](const QString text) {
                if(text.isEmpty())
                    _ui.appendTimestampCheckBox->setChecked(false);
                validateBackupTab();
            });

    // Bottom-right button
    _ui.backupButton->setDefaultAction(_ui.actionBackupNow);
    _ui.backupButton->addAction(_ui.actionBackupMorphIntoJob);
    connect(_ui.actionBackupNow, &QAction::triggered, this,
            &BackupTabWidget::backupButtonClicked);
    connect(_ui.actionBackupMorphIntoJob, &QAction::triggered, this,
            &BackupTabWidget::backupMorphIntoJobClicked);
}

void BackupTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui.retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void BackupTabWidget::updateUi()
{
    _ui.backupListInfoLabel->setToolTip(_ui.backupListInfoLabel->toolTip().arg(
        _ui.actionBrowseItems->shortcut().toString(QKeySequence::NativeText)));
    _ui.backupListInfoLabel->setText(_ui.backupListInfoLabel->text().arg(
        _ui.actionBrowseItems->shortcut().toString(QKeySequence::NativeText)));
}

bool BackupTabWidget::validateBackupTab()
{
    if(!_ui.backupNameLineEdit->text().isEmpty())
        return true;
    else
        return false;
}

void BackupTabWidget::backupTabValidStatus(bool valid)
{
    _ui.actionBackupNow->setEnabled(valid);
    _ui.actionBackupMorphIntoJob->setEnabled(valid);
}

void BackupTabWidget::appendTimestampCheckBoxToggled(bool checked)
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

void BackupTabWidget::updateBackupItemTotals(quint64 count, quint64 size)
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

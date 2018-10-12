#include "backuptabwidget.h"

#include "persistentmodel/archive.h"

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
}

bool BackupTabWidget::validateBackupTab()
{
    if(!_ui.backupNameLineEdit->text().isEmpty())
        return true;
    else
        return false;
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

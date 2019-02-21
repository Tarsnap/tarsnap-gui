#include "backuptabwidget.h"

#include "backuplistwidgetitem.h"
#include "persistentmodel/archive.h"
#include "utils.h"

#include "ui_backuptabwidget.h"

#include <QDateTime>
#include <QFileDialog>
#include <QSettings>
#include <QWidget>

// HACK: using _filePickerDialog(this) produces a segfault on MacOS X in
// QTabBarPrivate::updateMacBorderMetrics() with tests/mainwindow when using
// -platform offscreen.  I have no idea why, but I have no leads left to
// investigate and I've spent way longer than I expected on this refactoring,
// so I'm moving on.
BackupTabWidget::BackupTabWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::BackupTabWidget), _filePickerDialog(parent)
{
    // Ui initialization
    _ui->setupUi(this);
    _ui->backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    // Messages between widgets on this tab
    connect(_ui->appendTimestampCheckBox, &QCheckBox::toggled, this,
            &BackupTabWidget::appendTimestampCheckBoxToggled);
    connect(_ui->backupListInfoLabel, &ElidedLabel::clicked,
            _ui->actionBrowseItems, &QAction::trigger);
    connect(_ui->backupNameLineEdit, &QLineEdit::textChanged,
            [&](const QString text) {
                if(text.isEmpty())
                    _ui->appendTimestampCheckBox->setChecked(false);
                validateBackupTab();
            });
    connect(_ui->backupListWidget, &BackupListWidget::itemWithUrlAdded,
            &_filePickerDialog, &FilePickerDialog::selectUrl);
    connect(_ui->backupListWidget, &BackupListWidget::itemTotals, this,
            &BackupTabWidget::updateBackupItemTotals);

    // Bottom-right button
    _ui->backupButton->setDefaultAction(_ui->actionBackupNow);
    _ui->backupButton->addAction(_ui->actionBackupMorphIntoJob);
    connect(_ui->actionBackupNow, &QAction::triggered, this,
            &BackupTabWidget::backupButtonClicked);
    connect(_ui->actionBackupMorphIntoJob, &QAction::triggered, this,
            &BackupTabWidget::backupMorphIntoJobClicked);

    // Right-click context menu
    _ui->backupListWidget->addAction(_ui->actionBrowseItems);
    _ui->backupListWidget->addAction(_ui->actionAddFiles);
    _ui->backupListWidget->addAction(_ui->actionAddDirectory);
    _ui->backupListWidget->addAction(_ui->actionClearList);

    // Handle the Backup-related actions
    connect(_ui->actionBrowseItems, &QAction::triggered, this,
            &BackupTabWidget::browseForBackupItems);
    connect(_ui->actionAddFiles, &QAction::triggered, this,
            &BackupTabWidget::addFiles);
    connect(_ui->actionAddDirectory, &QAction::triggered, this,
            &BackupTabWidget::addDirectory);
    connect(_ui->actionClearList, &QAction::triggered, this,
            &BackupTabWidget::clearList);

    updateUi();
}

BackupTabWidget::~BackupTabWidget()
{
    delete _ui;
}

void BackupTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void BackupTabWidget::updateUi()
{
    _ui->backupListInfoLabel->setToolTip(_ui->backupListInfoLabel->toolTip().arg(
        _ui->actionBrowseItems->shortcut().toString(QKeySequence::NativeText)));
    _ui->backupListInfoLabel->setText(_ui->backupListInfoLabel->text().arg(
        _ui->actionBrowseItems->shortcut().toString(QKeySequence::NativeText)));
}

void BackupTabWidget::validateBackupTab()
{
    bool    valid = true;
    QString name  = _ui->backupNameLineEdit->text();

    // We need a name and at least one item
    if(name.isEmpty() || (_ui->backupListWidget->count() == 0))
        valid = false;

    // Check that we don't have any leading or trailing whitespace
    if(name.simplified() != name)
        valid = false;

    _ui->actionBackupNow->setEnabled(valid);
    _ui->actionBackupMorphIntoJob->setEnabled(valid);
    emit backupTabValidStatus(valid);
}

void BackupTabWidget::appendTimestampCheckBoxToggled(bool checked)
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

void BackupTabWidget::updateBackupItemTotals(quint64 count, quint64 size)
{
    if(count != 0)
    {
        _ui->backupDetailLabel->setText(
            tr("%1 %2 (%3)")
                .arg(count)
                .arg(count == 1 ? tr("item") : tr("items"))
                .arg(Utils::humanBytes(size)));
    }
    else
    {
        _ui->backupDetailLabel->clear();
    }
    validateBackupTab();
}

void BackupTabWidget::backupMorphIntoJobClicked()
{
    emit morphBackupIntoJob(_ui->backupListWidget->itemUrls(),
                            _ui->backupNameLineEdit->text());
}

void BackupTabWidget::backupButtonClicked()
{
    QList<QUrl> urls;
    for(int i = 0; i < _ui->backupListWidget->count(); ++i)
        urls << static_cast<BackupListWidgetItem *>(
                    _ui->backupListWidget->item(i))
                    ->url();

    BackupTaskPtr backup(new BackupTask);
    backup->setName(_ui->backupNameLineEdit->text());
    backup->setUrls(urls);
    emit backupNow(backup);
    _ui->appendTimestampCheckBox->setChecked(false);
}

void BackupTabWidget::browseForBackupItems()
{
    _filePickerDialog.setSelectedUrls(_ui->backupListWidget->itemUrls());
    if(_filePickerDialog.exec())
        _ui->backupListWidget->setItemsWithUrls(
            _filePickerDialog.getSelectedUrls());
}

void BackupTabWidget::addFiles()
{
    QList<QUrl> urls = QFileDialog::getOpenFileUrls(
        this, tr("Browse for files to add to the Backup list"));
    if(urls.count())
        _ui->backupListWidget->addItemsWithUrls(urls);
}

void BackupTabWidget::addDirectory()
{
    QUrl url = QFileDialog::getExistingDirectoryUrl(
        this, tr("Browse for directory to add to the Backup list"));
    if(!url.isEmpty())
        _ui->backupListWidget->addItemWithUrl(url);
}

void BackupTabWidget::clearList()
{
    _ui->backupListWidget->clear();
}

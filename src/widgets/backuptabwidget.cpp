#include "backuptabwidget.h"

WARNINGS_DISABLE
#include <QCheckBox>
#include <QDateTime>
#include <QDialog>
#include <QEvent>
#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <Qt>

#include "ui_backuptabwidget.h"
WARNINGS_ENABLE

#include "backuptask.h"
#include "basetask.h"
#include "humanbytes.h"
#include "persistentmodel/archive.h"
#include "widgets/backuplistwidget.h"
#include "widgets/backuplistwidgetitem.h"
#include "widgets/elidedclickablelabel.h"
#include "widgets/filepickerdialog.h"

// HACK: using _filePickerDialog(this) produces a segfault on MacOS X in
// QTabBarPrivate::updateMacBorderMetrics() with tests/mainwindow when using
// platform=offscreen.  I have no idea why, but I have no leads left to
// investigate and I've spent way longer than I expected on this refactoring,
// so I'm moving on.
BackupTabWidget::BackupTabWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::BackupTabWidget),
      _filePickerDialog(new FilePickerDialog(parent)),
      _count(0),
      _size(0)
{
    // Ui initialization
    _ui->setupUi(this);
    updateKeyboardShortcutInfo();
    _ui->backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    // Messages between widgets on this tab
    connect(_ui->appendTimestampCheckBox, &QCheckBox::toggled, this,
            &BackupTabWidget::appendTimestampCheckBoxToggled);
    connect(_ui->backupListInfoLabel, &ElidedClickableLabel::clicked,
            _ui->actionBrowseItems, &QAction::trigger);
    connect(_ui->backupNameLineEdit, &QLineEdit::textChanged,
            [this](const QString &text) {
                if(text.isEmpty())
                    _ui->appendTimestampCheckBox->setChecked(false);
                validateBackupTab();
            });
    connect(_ui->backupListWidget, &BackupListWidget::itemWithUrlAdded,
            _filePickerDialog, &FilePickerDialog::selectUrl);
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

    // Allow the FilePickerDialog to use open()
    connect(_filePickerDialog, &FilePickerDialog::finished, this,
            &BackupTabWidget::processFPD);

    // Pass messages about tasks.
    connect(_ui->backupListWidget, &BackupListWidget::taskRequested, this,
            &BackupTabWidget::taskRequested);
    connect(_ui->backupListWidget, &BackupListWidget::cancelTaskRequested, this,
            &BackupTabWidget::cancelTaskRequested);
}

BackupTabWidget::~BackupTabWidget()
{
    delete _filePickerDialog;
    delete _ui;
}

void BackupTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateKeyboardShortcutInfo();
    }
    QWidget::changeEvent(event);
}

void BackupTabWidget::updateKeyboardShortcutInfo()
{
    _ui->backupListInfoLabel->setToolTip(
        _ui->backupListInfoLabel->toolTip().arg(
            _ui->actionBrowseItems->shortcut().toString(
                QKeySequence::NativeText)));
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

    // Act on the validity.
    _ui->actionBackupNow->setEnabled(valid);
    _ui->actionBackupMorphIntoJob->setEnabled(valid);
    emit backupTabValidStatus(valid);
}

void BackupTabWidget::appendTimestampCheckBoxToggled(bool checked)
{
    if(checked)
    {
        // Get backup name.
        QString text = _ui->backupNameLineEdit->text();

        // Append current time to text.
        _lastTimestamp.clear();
        _lastTimestamp.append(
            QDateTime::currentDateTime().toString(ARCHIVE_TIMESTAMP_FORMAT));
        text.append(_lastTimestamp);

        // Set backup name.
        _ui->backupNameLineEdit->setText(text);
        _ui->backupNameLineEdit->setCursorPosition(0);
    }
    else
    {
        // Get backup name.
        QString text = _ui->backupNameLineEdit->text();

        // Remove timestamp (if it matches the saved timestamp string).
        if(!_lastTimestamp.isEmpty() && text.endsWith(_lastTimestamp))
        {
            text.chop(_lastTimestamp.length());
            _ui->backupNameLineEdit->setText(text);
        }
    }
}

void BackupTabWidget::updateBackupItemTotals(quint64 count, quint64 size)
{
    _count = count;
    _size  = size;
    updateIEC();
    validateBackupTab();
}

void BackupTabWidget::updateIEC()
{
    // Pass message to list
    _ui->backupListWidget->updateIEC();

    if(_count != 0)
    {
        _ui->backupDetailLabel->setText(
            tr("%1 %2 (%3)")
                .arg(_count)
                .arg(_count == 1 ? tr("item") : tr("items"))
                .arg(humanBytes(_size)));
    }
    else
    {
        _ui->backupDetailLabel->clear();
    }
}

void BackupTabWidget::backupMorphIntoJobClicked()
{
    emit morphBackupIntoJob(_ui->backupListWidget->itemUrls(),
                            _ui->backupNameLineEdit->text());
}

void BackupTabWidget::backupButtonClicked()
{
    // Get a list of the URLs.
    QList<QUrl> urls;
    for(int i = 0; i < _ui->backupListWidget->count(); ++i)
        urls << static_cast<BackupListWidgetItem *>(
                    _ui->backupListWidget->item(i))
                    ->url();

    // Make a new BackupTaskData with info from this tab.
    BackupTaskDataPtr backup(new BackupTaskData);
    backup->setName(_ui->backupNameLineEdit->text());
    backup->setUrls(urls);

    // Start creating a new archive.
    emit backupNow(backup);

    // Clear the timestamp checkbox.
    _ui->appendTimestampCheckBox->setChecked(false);
}

void BackupTabWidget::browseForBackupItems()
{
    _filePickerDialog->setSelectedUrls(_ui->backupListWidget->itemUrls());
    _filePickerDialog->open();
}

void BackupTabWidget::processFPD(int res)
{
    // Bail (if appropriate).
    if(res != QDialog::Accepted)
        return;

    // Set the current items to those in the FilePickerDialog.
    _ui->backupListWidget->setItemsWithUrls(
        _filePickerDialog->getSelectedUrls());
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

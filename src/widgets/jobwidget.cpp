#include "jobwidget.h"
#include "debug.h"
#include "restoredialog.h"
#include "utils.h"

#include <QMenu>

JobWidget::JobWidget(QWidget *parent)
    : QWidget(parent), _saveEnabled(false)
{
    _ui.setupUi(this);
    _ui.archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui.infoLabel->hide();
    connect(_ui.infoLabel, &TextLabel::clicked, this, &JobWidget::updateDetails);
    connect(_ui.jobNameLineEdit, &QLineEdit::textChanged, [&]() {
        if(_job->objectKey().isEmpty())
            emit enableSave(canSaveNew());
    });
    connect(_ui.jobTreeWidget, &FilePickerWidget::selectionChanged, [&]() {
        if(_job->objectKey().isEmpty())
            emit enableSave(canSaveNew());
        else
            save();
    });
    connect(_ui.jobTreeWidget, &FilePickerWidget::settingChanged, [&]() {
        if(!_job->objectKey().isEmpty())
            save();
    });

    connect(_ui.includeScheduledCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui.preservePathsCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui.traverseMountCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui.followSymLinksCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui.skipNoDumpCheckBox, &QCheckBox::toggled, this, &JobWidget::save);
    connect(_ui.skipFilesSizeSpinBox, &QSpinBox::editingFinished, this,
            &JobWidget::save);
    connect(_ui.skipFilesCheckBox, &QCheckBox::toggled, this, &JobWidget::save);
    connect(_ui.skipFilesLineEdit, &QLineEdit::editingFinished, this,
            &JobWidget::save);
    connect(_ui.hideButton, &QPushButton::clicked, this, &JobWidget::collapse);
    connect(_ui.restoreButton, &QPushButton::clicked, this,
            &JobWidget::restoreButtonClicked);
    connect(_ui.backupButton, &QPushButton::clicked, this,
            &JobWidget::backupButtonClicked);
    connect(_ui.archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &JobWidget::inspectJobArchive);
    connect(_ui.archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &JobWidget::restoreJobArchive);
    connect(_ui.archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &JobWidget::deleteJobArchives);
    connect(_ui.skipFilesDefaultsButton, &QPushButton::clicked, [&]() {
        QSettings settings;
        _ui.skipFilesLineEdit->setText(
            settings.value("app/skip_system_files", DEFAULT_SKIP_FILES).toString());
    });
    connect(_ui.archiveListWidget,
            &ArchiveListWidget::customContextMenuRequested, this,
            &JobWidget::showArchiveListMenu);
    connect(_ui.actionDelete, &QAction::triggered, _ui.archiveListWidget,
            &ArchiveListWidget::removeSelectedItems);
    connect(_ui.actionRestore, &QAction::triggered, _ui.archiveListWidget,
            &ArchiveListWidget::restoreSelectedItem);
    connect(_ui.actionInspect, &QAction::triggered, _ui.archiveListWidget,
            &ArchiveListWidget::inspectSelectedItem);
}

JobWidget::~JobWidget()
{
}
JobPtr JobWidget::job() const
{
    return _job;
}

void JobWidget::setJob(const JobPtr &job)
{
    if(_job)
    {
        _job->removeWatcher();
        disconnect(_job.data(), &Job::changed, this, &JobWidget::updateDetails);
    }

    _saveEnabled = false;
    _job = job;

    // Creating a new job?
    if(_job->objectKey().isEmpty())
    {
        _ui.tabWidget->setTabEnabled(_ui.tabWidget->indexOf(_ui.archiveListTab),
                                      false);
        _ui.restoreButton->hide();
        _ui.backupButton->hide();
        _ui.jobNameLabel->hide();
        _ui.jobNameLineEdit->show();
        _ui.jobNameLineEdit->setFocus();
    }
    else
    {
        _ui.tabWidget->setTabEnabled(_ui.tabWidget->indexOf(_ui.archiveListTab),
                                      true);
        _ui.restoreButton->show();
        _ui.backupButton->show();
        _ui.jobNameLabel->show();
        _ui.jobNameLineEdit->hide();
        connect(_job.data(), &Job::changed, this, &JobWidget::updateDetails);
        connect(_job.data(), &Job::pathsChanged, this, &JobWidget::verifyJobPaths);
        job->installWatcher();
    }
    _ui.tabWidget->setCurrentWidget(_ui.jobTreeTab);
    updateDetails();
    _saveEnabled = true;
}

void JobWidget::save()
{
    if(_saveEnabled && !_job->name().isEmpty())
    {
        DEBUG << "SAVE JOB";
        _job->setUrls(_ui.jobTreeWidget->getSelectedUrls());
        _job->setOptionScheduledEnabled(
            _ui.includeScheduledCheckBox->isChecked());
        _job->setOptionPreservePaths(_ui.preservePathsCheckBox->isChecked());
        _job->setOptionTraverseMount(_ui.traverseMountCheckBox->isChecked());
        _job->setOptionFollowSymLinks(_ui.followSymLinksCheckBox->isChecked());
        _job->setOptionSkipNoDump(_ui.skipNoDumpCheckBox->isChecked());
        _job->setOptionSkipFilesSize(_ui.skipFilesSizeSpinBox->value());
        _job->setOptionSkipFiles(_ui.skipFilesCheckBox->isChecked());
        _job->setOptionSkipFilesPatterns(_ui.skipFilesLineEdit->text());
        _job->setSettingShowHidden(_ui.jobTreeWidget->settingShowHidden());
        _job->setSettingShowSystem(_ui.jobTreeWidget->settingShowSystem());
        _job->setSettingHideSymlinks(_ui.jobTreeWidget->settingHideSymlinks());
        _job->save();
    }
}

void JobWidget::saveNew()
{
    if(canSaveNew())
    {
        DEBUG << "SAVE NEW JOB";
        _job->setName(_ui.jobNameLineEdit->text());
        save();
        emit jobAdded(_job);
    }
}

void JobWidget::updateDetails()
{
    if(!_job)
        return;
    DEBUG << "UPDATE JOB DETAILS";
    _saveEnabled = false;
    _ui.jobNameLabel->setText(_job->name());
    _ui.jobTreeWidget->setSettingShowHidden(_job->settingShowHidden());
    _ui.jobTreeWidget->setSettingShowSystem(_job->settingShowSystem());
    _ui.jobTreeWidget->setSettingHideSymlinks(_job->settingHideSymlinks());
    _ui.jobTreeWidget->blockSignals(true);
    _ui.jobTreeWidget->setSelectedUrls(_job->urls());
    _ui.jobTreeWidget->blockSignals(false);
    _ui.infoLabel->setVisible(!_job->validateUrls());
    if(_ui.infoLabel->isVisible())
    {
        QStringList urls;
        foreach(QUrl url, _job->urls())
            urls << url.toLocalFile();
        _ui.infoLabel->setToolTip(urls.join('\n'));
    }
    _ui.archiveListWidget->clear();
    _ui.archiveListWidget->addArchives(_job->archives());
    _ui.includeScheduledCheckBox->setChecked(_job->optionScheduledEnabled());
    _ui.preservePathsCheckBox->setChecked(_job->optionPreservePaths());
    _ui.traverseMountCheckBox->setChecked(_job->optionTraverseMount());
    _ui.followSymLinksCheckBox->setChecked(_job->optionFollowSymLinks());
    _ui.skipNoDumpCheckBox->setChecked(_job->optionSkipNoDump());
    _ui.skipFilesSizeSpinBox->setValue(_job->optionSkipFilesSize());
    _ui.skipFilesCheckBox->setChecked(_job->optionSkipFiles());
    _ui.skipFilesLineEdit->setText(_job->optionSkipFilesPatterns());
    _ui.tabWidget->setTabText(_ui.tabWidget->indexOf(_ui.archiveListTab),
                              tr("Archives (%1)").arg(_job->archives().count()));
    _saveEnabled = true;
}

void JobWidget::restoreButtonClicked()
{
    if(_job && !_job->archives().isEmpty())
    {
        ArchivePtr    archive = _job->archives().first();
        RestoreDialog restoreDialog(archive, this);
        if(QDialog::Accepted == restoreDialog.exec())
            emit restoreJobArchive(archive, restoreDialog.getOptions());
    }
}

void JobWidget::backupButtonClicked()
{
    if(_job)
        emit backupJob(_job->createBackupTask());
}

bool JobWidget::canSaveNew()
{
    if(_job->objectKey().isEmpty() && !_ui.jobNameLineEdit->text().isEmpty() &&
       !_ui.jobTreeWidget->getSelectedUrls().isEmpty())
        return true;
    else
        return false;
}

void JobWidget::showArchiveListMenu(const QPoint &pos)
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
    archiveListMenu.exec(globalPos);
}

void JobWidget::verifyJobPaths()
{
    // change to instance timer and coalesce events
    QTimer::singleShot(100, this, &JobWidget::updateDetails);
}

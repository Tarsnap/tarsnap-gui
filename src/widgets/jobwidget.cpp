#include "jobwidget.h"
#include "debug.h"
#include "restoredialog.h"
#include "ui_jobwidget.h"
#include "utils.h"

#include <QMenu>

JobWidget::JobWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::JobWidget), _saveEnabled(false)
{
    _ui->setupUi(this);
    _ui->archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(_ui->jobNameLineEdit, &QLineEdit::textChanged, [&]() {
        if(_job->objectKey().isEmpty())
            emit enableSave(canSaveNew());
    });
    connect(_ui->jobTreeWidget, &FilePickerWidget::selectionChanged, [&]() {
        if(_job->objectKey().isEmpty())
            emit enableSave(canSaveNew());
        else
            save();
    });
//    connect(_ui->jobTreeWidget, &FilePickerWidget::focusLost,
//            [&](){
//                    if(!_job->objectKey().isEmpty())
//                        save();
//            });

    connect(_ui->includeScheduledCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui->preservePathsCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui->traverseMountCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui->followSymLinksCheckBox, &QCheckBox::toggled, this,
            &JobWidget::save);
    connect(_ui->skipNoDumpCheckBox, &QCheckBox::toggled, this, &JobWidget::save);
    connect(_ui->skipFilesSizeSpinBox, &QSpinBox::editingFinished, this,
            &JobWidget::save);
    connect(_ui->skipFilesCheckBox, &QCheckBox::toggled, this, &JobWidget::save);
    connect(_ui->skipFilesLineEdit, &QLineEdit::editingFinished, this,
            &JobWidget::save);
    connect(_ui->hideButton, &QPushButton::clicked, this, &JobWidget::collapse);
    connect(_ui->restoreLatestArchiveButton, &QPushButton::clicked, this,
            &JobWidget::restoreLatestArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &JobWidget::inspectJobArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &JobWidget::restoreJobArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &JobWidget::deleteJobArchives);
    connect(_ui->skipFilesDefaultsButton, &QPushButton::clicked, [&]() {
        QSettings settings;
        _ui->skipFilesLineEdit->setText(
            settings.value("app/skip_system_files", DEFAULT_SKIP_FILES).toString());
    });
    connect(_ui->archiveListWidget,
            &ArchiveListWidget::customContextMenuRequested, this,
            &JobWidget::showArchiveListMenu);
    connect(_ui->actionDelete, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::removeSelectedItems);
    connect(_ui->actionRestore, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::restoreSelectedItem);
    connect(_ui->actionInspect, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::inspectSelectedItem);
}

JobWidget::~JobWidget()
{
    delete _ui;
}
JobPtr JobWidget::job() const
{
    return _job;
}

void JobWidget::setJob(const JobPtr &job)
{
    _job = job;

    // Creating a new job?
    if(_job->objectKey().isEmpty())
    {
        _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(_ui->archiveListTab),
                                      false);
        _ui->restoreLatestArchiveButton->hide();
        _ui->jobNameLabel->hide();
        _ui->jobNameLineEdit->show();
        _ui->tabWidget->setCurrentWidget(_ui->jobTreeTab);
        _ui->jobNameLineEdit->setFocus();
        QSettings settings;
        _ui->includeScheduledCheckBox->setChecked(false);
        _ui->preservePathsCheckBox->setChecked(
            settings.value("tarsnap/preserve_pathnames", true).toBool());
        _ui->traverseMountCheckBox->setChecked(
            settings.value("tarsnap/traverse_mount", true).toBool());
        _ui->followSymLinksCheckBox->setChecked(
            settings.value("tarsnap/follow_symlinks", false).toBool());
        _ui->skipNoDumpCheckBox->setChecked(
            settings.value("app/skip_nodump", false).toBool());
        _ui->skipFilesSizeSpinBox->setValue(
            settings.value("app/skip_files_size", 0).toInt());
        _ui->skipFilesCheckBox->setChecked(
            settings.value("app/skip_system_enabled", false).toBool());
        _ui->skipFilesLineEdit->setText(
            settings.value("app/skip_system_files", DEFAULT_SKIP_FILES).toString());
    }
    else
    {
        _saveEnabled = false;
        _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(_ui->archiveListTab),
                                      true);
        _ui->restoreLatestArchiveButton->show();
        _ui->jobNameLabel->show();
        _ui->jobNameLineEdit->hide();
        _ui->tabWidget->setCurrentWidget(_ui->jobTreeTab);
        connect(_job.data(), &Job::changed, this, &JobWidget::updateDetails);
        _saveEnabled = true;
    }
    updateDetails();
}

void JobWidget::save()
{
    if(_saveEnabled && !_job->objectKey().isEmpty())
    {
        DEBUG << "SAVE JOB";
        _job->setUrls(_ui->jobTreeWidget->getSelectedUrls());
        _job->setOptionScheduledEnabled(
            _ui->includeScheduledCheckBox->isChecked());
        _job->setOptionPreservePaths(_ui->preservePathsCheckBox->isChecked());
        _job->setOptionTraverseMount(_ui->traverseMountCheckBox->isChecked());
        _job->setOptionFollowSymLinks(_ui->followSymLinksCheckBox->isChecked());
        _job->setOptionSkipNoDump(_ui->skipNoDumpCheckBox->isChecked());
        _job->setOptionSkipFilesSize(_ui->skipFilesSizeSpinBox->value());
        _job->setOptionSkipFiles(_ui->skipFilesCheckBox->isChecked());
        _job->setOptionSkipFilesPatterns(_ui->skipFilesLineEdit->text());
        _job->save();
    }
}

void JobWidget::saveNew()
{
    if(canSaveNew())
    {
        DEBUG << "SAVE NEW JOB";
        _job->setName(_ui->jobNameLineEdit->text());
        _job->setUrls(_ui->jobTreeWidget->getSelectedUrls());
        _job->setOptionScheduledEnabled(
            _ui->includeScheduledCheckBox->isChecked());
        _job->setOptionPreservePaths(_ui->preservePathsCheckBox->isChecked());
        _job->setOptionTraverseMount(_ui->traverseMountCheckBox->isChecked());
        _job->setOptionFollowSymLinks(_ui->followSymLinksCheckBox->isChecked());
        _job->setOptionSkipNoDump(_ui->skipNoDumpCheckBox->isChecked());
        _job->setOptionSkipFilesSize(_ui->skipFilesSizeSpinBox->value());
        _job->setOptionSkipFiles(_ui->skipFilesCheckBox->isChecked());
        _job->setOptionSkipFilesPatterns(_ui->skipFilesLineEdit->text());
        _job->save();
        emit jobAdded(_job);
    }
}

void JobWidget::updateDetails()
{
    if(_job)
    {
        _ui->jobNameLineEdit->setText(_job->name());
        _ui->jobNameLabel->setText(_job->name());
        _ui->jobTreeWidget->blockSignals(true);
        _ui->jobTreeWidget->setSelectedUrls(_job->urls());
        _ui->jobTreeWidget->blockSignals(false);
        _ui->archiveListWidget->clear();
        _ui->archiveListWidget->addArchives(_job->archives());
        _ui->includeScheduledCheckBox->setChecked(_job->optionScheduledEnabled());
        _ui->preservePathsCheckBox->setChecked(_job->optionPreservePaths());
        _ui->traverseMountCheckBox->setChecked(_job->optionTraverseMount());
        _ui->followSymLinksCheckBox->setChecked(_job->optionFollowSymLinks());
        _ui->skipNoDumpCheckBox->setChecked(_job->optionSkipNoDump());
        _ui->skipFilesSizeSpinBox->setValue(_job->optionSkipFilesSize());
        _ui->skipFilesCheckBox->setChecked(_job->optionSkipFiles());
        _ui->skipFilesLineEdit->setText(_job->optionSkipFilesPatterns());
    }
}

void JobWidget::restoreLatestArchive()
{
    if(_job && !_job->archives().isEmpty())
    {
        ArchivePtr    archive = _job->archives().first();
        RestoreDialog restoreDialog(archive, this);
        if(QDialog::Accepted == restoreDialog.exec())
            emit restoreJobArchive(archive, restoreDialog.getOptions());
    }
}

bool JobWidget::canSaveNew()
{
    if(_job->objectKey().isEmpty() && !_ui->jobNameLineEdit->text().isEmpty() &&
       !_ui->jobTreeWidget->getSelectedUrls().isEmpty())
        return true;
    else
        return false;
}

void JobWidget::showArchiveListMenu(const QPoint &pos)
{
    QPoint globalPos = _ui->archiveListWidget->viewport()->mapToGlobal(pos);
    QMenu  archiveListMenu(_ui->archiveListWidget);
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

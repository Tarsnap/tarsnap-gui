#include "jobwidget.h"
#include "ui_jobwidget.h"
#include "restoredialog.h"
#include "debug.h"
#include "utils.h"

JobWidget::JobWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::JobWidget),
    _saveEnabled(false)
{
    _ui->setupUi(this);
    _ui->archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(_ui->jobNameLineEdit, &QLineEdit::textChanged,
    [=]() {
        if(_job->objectKey().isEmpty())
            emit enableSave(canSaveNew());
    });
    connect(_ui->jobTreeWidget, &FilePicker::selectionChanged,
    [=]() {
        if(_job->objectKey().isEmpty())
            emit enableSave(canSaveNew());
        else
            save();
    });
//    connect(_ui->jobTreeWidget, &FilePicker::focusLost,
//            [=](){
//                    if(!_job->objectKey().isEmpty())
//                        save();
//            });

    connect(_ui->includeScheduledCheckBox, SIGNAL(toggled(bool)), this, SLOT(save()));
    connect(_ui->preservePathsCheckBox, SIGNAL(toggled(bool)), this, SLOT(save()));
    connect(_ui->traverseMountCheckBox, SIGNAL(toggled(bool)), this, SLOT(save()));
    connect(_ui->followSymLinksCheckBox, SIGNAL(toggled(bool)), this, SLOT(save()));
    connect(_ui->skipFilesSizeSpinBox, SIGNAL(editingFinished()), this, SLOT(save()));
    connect(_ui->skipFilesCheckBox, SIGNAL(toggled(bool)), this, SLOT(save()));
    connect(_ui->skipFilesLineEdit, SIGNAL(editingFinished()), this, SLOT(save()));
    connect(_ui->cancelButton, SIGNAL(clicked()), this, SIGNAL(cancel()));
    connect(_ui->restoreLatestArchiveButton, SIGNAL(clicked()), this,
            SLOT(restoreLatestArchive()));
    connect(_ui->archiveListWidget, SIGNAL(inspectArchive(ArchivePtr)), this,
            SIGNAL(inspectJobArchive(ArchivePtr)));
    connect(_ui->archiveListWidget, SIGNAL(restoreArchive(ArchivePtr, ArchiveRestoreOptions)),
            this, SIGNAL(restoreJobArchive(ArchivePtr, ArchiveRestoreOptions)));
    connect(_ui->archiveListWidget, SIGNAL(deleteArchives(QList<ArchivePtr>)),
            this, SIGNAL(deleteJobArchives(QList<ArchivePtr>)));
    connect(_ui->skipFilesDefaultsButton, &QPushButton::clicked,
    [=]() {
        QSettings settings;
        _ui->skipFilesLineEdit->setText(settings.value("app/skip_system_files",
                                                       DEFAULT_SKIP_FILES).toString());
    });
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
        _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(_ui->archiveListTab), false);
        _ui->restoreLatestArchiveButton->hide();
        _ui->jobNameLabel->hide();
        _ui->jobNameLineEdit->show();
        _ui->jobTreeWidget->blockSignals(true);
        _ui->jobTreeWidget->reset();
        _ui->jobTreeWidget->blockSignals(false);
        _ui->tabWidget->setCurrentWidget(_ui->jobTreeTab);
        _ui->jobNameLineEdit->setFocus();
        QSettings settings;
        _ui->includeScheduledCheckBox->setChecked(false);
        _ui->preservePathsCheckBox->setChecked(settings.value("tarsnap/preserve_pathnames", true).toBool());
        _ui->traverseMountCheckBox->setChecked(settings.value("tarsnap/traverse_mount", true).toBool());
        _ui->followSymLinksCheckBox->setChecked(settings.value("tarsnap/follow_symlinks", false).toBool());
        _ui->skipFilesSizeSpinBox->setValue(settings.value("app/skip_files_size", 0).toLongLong());
        _ui->skipFilesCheckBox->setChecked(settings.value("app/skip_system_enabled", false).toBool());
        _ui->skipFilesLineEdit->setText(settings.value("app/skip_system_files", DEFAULT_SKIP_FILES).toString());
    }
    else
    {
        _saveEnabled = false;
        _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(_ui->archiveListTab), true);
        _ui->restoreLatestArchiveButton->show();
        _ui->jobNameLabel->show();
        _ui->jobNameLineEdit->hide();
        _ui->tabWidget->setCurrentWidget(_ui->jobTreeTab);
        updateDetails();
        connect(_job.data(), SIGNAL(changed()), this, SLOT(updateDetails()));
        _saveEnabled = true;
    }
}

void JobWidget::save()
{
    if(_saveEnabled && !_job->objectKey().isEmpty())
    {
        DEBUG << "SAVE JOB";
        _job->setUrls(_ui->jobTreeWidget->getSelectedUrls());
        _job->setOptionScheduledEnabled(_ui->includeScheduledCheckBox->isChecked());
        _job->setOptionPreservePaths(_ui->preservePathsCheckBox->isChecked());
        _job->setOptionTraverseMount(_ui->traverseMountCheckBox->isChecked());
        _job->setOptionFollowSymLinks(_ui->followSymLinksCheckBox->isChecked());
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
        _job->setOptionScheduledEnabled(_ui->includeScheduledCheckBox->isChecked());
        _job->setOptionPreservePaths(_ui->preservePathsCheckBox->isChecked());
        _job->setOptionTraverseMount(_ui->traverseMountCheckBox->isChecked());
        _job->setOptionFollowSymLinks(_ui->followSymLinksCheckBox->isChecked());
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
        _ui->skipFilesSizeSpinBox->setValue(_job->optionSkipFilesSize());
        _ui->skipFilesCheckBox->setChecked(_job->optionSkipFiles());
        _ui->skipFilesLineEdit->setText(_job->optionSkipFilesPatterns());
    }
}

void JobWidget::restoreLatestArchive()
{
    if(_job && !_job->archives().isEmpty())
    {
        ArchivePtr archive = _job->archives().first();
        RestoreDialog restoreDialog(archive, this);
        if( QDialog::Accepted == restoreDialog.exec())
            emit restoreJobArchive(archive, restoreDialog.getOptions());
    }
}

bool JobWidget::canSaveNew()
{
    if(_job->objectKey().isEmpty() && !_ui->jobNameLineEdit->text().isEmpty()
       && !_ui->jobTreeWidget->getSelectedUrls().isEmpty())
        return true;
    else
        return false;
}


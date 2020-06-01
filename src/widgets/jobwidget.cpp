#include "jobwidget.h"

WARNINGS_DISABLE
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCursor>
#include <QEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QTimer>
#include <QUrl>
#include <QVariant>
#include <Qt>

#include "ui_jobwidget.h"
WARNINGS_ENABLE

#include "TSettings.h"

#include "messages/archiverestoreoptions.h"

#include "debug.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "tasks/tasks-defs.h"
#include "widgets/archivelistwidget.h"
#include "widgets/elidedclickablelabel.h"
#include "widgets/filepickerwidget.h"
#include "widgets/restoredialog.h"

JobDetailsWidget::JobDetailsWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::JobDetailsWidget),
      _saveEnabled(false),
      _fsEventUpdate(new QTimer(this))
{
    _ui->setupUi(this);

    // Basic UI.
    _ui->archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    updateKeyboardShortcutInfo();
    _archiveListMenu = new QMenu(_ui->archiveListWidget);

    // Set up the timer for filesystem events.
    _fsEventUpdate->setSingleShot(true);
    connect(_fsEventUpdate, &QTimer::timeout, this,
            &JobDetailsWidget::verifyJob);

    // UI connections (simple).
    connect(_ui->infoLabel, &ElidedClickableLabel::clicked, this,
            &JobDetailsWidget::showJobPathsWarn);
    connect(_ui->hideButton, &QPushButton::clicked, this,
            &JobDetailsWidget::collapse);
    connect(_ui->restoreButton, &QPushButton::clicked, this,
            &JobDetailsWidget::restoreButtonClicked);
    connect(_ui->backupButton, &QPushButton::clicked, this,
            &JobDetailsWidget::backupButtonClicked);

    // UI connections (lambda).
    connect(_ui->jobNameLineEdit, &QLineEdit::textChanged,
            [this]() { emit enableSave(canSaveNew()); });
    connect(_ui->jobTreeWidget, &FilePickerWidget::selectionChanged, [this]() {
        if(_job->objectKey().isEmpty())
            emit enableSave(canSaveNew());
        else
            save();
    });
    connect(_ui->jobTreeWidget, &FilePickerWidget::settingChanged, [this]() {
        if(!_job->objectKey().isEmpty())
            save();
    });
    connect(_ui->skipFilesDefaultsButton, &QPushButton::clicked, [this]() {
        TSettings settings;
        _ui->skipFilesLineEdit->setText(
            settings.value("app/skip_system_files", DEFAULT_SKIP_SYSTEM_FILES)
                .toString());
    });

    // Save values to the Job.
    connect(_ui->scheduleComboBox,
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
            this, &JobDetailsWidget::save);
    connect(_ui->preservePathsCheckBox, &QCheckBox::toggled, this,
            &JobDetailsWidget::save);
    connect(_ui->traverseMountCheckBox, &QCheckBox::toggled, this,
            &JobDetailsWidget::save);
    connect(_ui->followSymLinksCheckBox, &QCheckBox::toggled, this,
            &JobDetailsWidget::save);
    connect(_ui->skipNoDumpCheckBox, &QCheckBox::toggled, this,
            &JobDetailsWidget::save);
    connect(_ui->skipFilesSizeSpinBox, &QSpinBox::editingFinished, this,
            &JobDetailsWidget::save);
    connect(_ui->skipFilesCheckBox, &QCheckBox::toggled, this,
            &JobDetailsWidget::save);
    connect(_ui->skipFilesLineEdit, &QLineEdit::editingFinished, this,
            &JobDetailsWidget::save);

    // Connection from archiveListWidget.
    connect(_ui->archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &JobDetailsWidget::inspectJobArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::inspectArchive,
            _ui->archiveListWidget, &ArchiveListWidget::selectArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &JobDetailsWidget::restoreJobArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &JobDetailsWidget::deleteJobArchives);
    connect(_ui->archiveListWidget,
            &ArchiveListWidget::customContextMenuRequested, this,
            &JobDetailsWidget::showArchiveListMenu);

    // Connections to archiveListWidget.
    connect(_ui->actionDelete, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::deleteSelectedItems);
    connect(_ui->actionRestore, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::restoreSelectedItem);
    connect(_ui->actionInspect, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::inspectSelectedItem);
}

JobDetailsWidget::~JobDetailsWidget()
{
    delete _ui;
}

JobPtr JobDetailsWidget::job() const
{
    return _job;
}

void JobDetailsWidget::setJob(const JobPtr &job)
{
    // Remove previous job (if applicable).
    if(_job)
    {
        _job->removeWatcher();
        disconnect(_job.data(), &Job::fsEvent, this,
                   &JobDetailsWidget::fsEventReceived);
        disconnect(_job.data(), &Job::changed, this,
                   &JobDetailsWidget::updateDetails);
        disconnect(_job.data(), &Job::purged, this,
                   &JobDetailsWidget::collapse);
    }

    // Job is not ready for saving yet.
    _saveEnabled = false;

    // Store pointer.
    _job = job;

    // Creating a new job?
    if(_job->objectKey().isEmpty())
    {
        // Set up UI for a new Job.
        _ui->restoreButton->hide();
        _ui->backupButton->hide();
        _ui->jobNameLabel->hide();

        // Prep the job name, ready for approval or editing from the user.
        _ui->jobNameLineEdit->setText(_job->name());
        _ui->jobNameLineEdit->show();
        _ui->jobNameLineEdit->setFocus();
    }
    else
    {
        // Set up UI for an existing Job.
        _ui->restoreButton->show();
        _ui->backupButton->show();
        _ui->jobNameLabel->show();
        _ui->jobNameLineEdit->hide();

        // Connections to handle any change in Job-related files or dirs.
        connect(_job.data(), &Job::changed, this,
                &JobDetailsWidget::updateDetails);
        connect(_job.data(), &Job::fsEvent, this,
                &JobDetailsWidget::fsEventReceived);
        connect(_job.data(), &Job::purged, this, &JobDetailsWidget::collapse);

        // Notify us if any change in Job-related files or dirs.
        job->installWatcher();
    }
    // Show the filesystem tree in the widget.
    _ui->tabWidget->setCurrentWidget(_ui->jobTreeTab);

    // Display details about the Job.
    updateDetails();

    // Job is ready for saving.
    _saveEnabled = true;
}

void JobDetailsWidget::save()
{
    // Bail if saving isn't appropriate.
    if(!_saveEnabled || _job->name().isEmpty())
        return;

    // Set urls and reset FileSystemWatcher.
    _job->setUrls(_ui->jobTreeWidget->getSelectedUrls());
    _job->removeWatcher();
    _job->installWatcher();

    // Set all other Job data (other than the name).
    _job->setOptionScheduledEnabled(
        static_cast<JobSchedule>(_ui->scheduleComboBox->currentIndex()));
    _job->setOptionPreservePaths(_ui->preservePathsCheckBox->isChecked());
    _job->setOptionTraverseMount(_ui->traverseMountCheckBox->isChecked());
    _job->setOptionFollowSymLinks(_ui->followSymLinksCheckBox->isChecked());
    _job->setOptionSkipNoDump(_ui->skipNoDumpCheckBox->isChecked());
    _job->setOptionSkipFilesSize(_ui->skipFilesSizeSpinBox->value());
    _job->setOptionSkipFiles(_ui->skipFilesCheckBox->isChecked());
    _job->setOptionSkipFilesPatterns(_ui->skipFilesLineEdit->text());
    _job->setSettingShowHidden(_ui->jobTreeWidget->settingShowHidden());
    _job->setSettingShowSystem(_ui->jobTreeWidget->settingShowSystem());
    _job->setSettingHideSymlinks(_ui->jobTreeWidget->settingHideSymlinks());

    // Save Job to global_store.
    _job->save();

    // Check that the URLs are valid, otherwise show an error message.
    verifyJob();
}

void JobDetailsWidget::saveNew()
{
    if(!canSaveNew())
        return;

    // Set the job name.
    _job->setName(_ui->jobNameLineEdit->text());

    // Assign matching archives to the Job (if the user agrees).
    if(!_job->archives().isEmpty())
    {
        QMessageBox::StandardButton confirm =
            QMessageBox::question(this, "Add job",
                                  tr("Assign %1 found archives to this Job?")
                                      .arg(_job->archives().count()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
        if(confirm == QMessageBox::Yes)
        {
            for(const ArchivePtr &archive : _job->archives())
            {
                archive->setJobRef(_job->objectKey());
                archive->save();
            }
        }
        else
        {
            QList<ArchivePtr> empty;
            _job->setArchives(empty);
        }
    }

    // Save and notify that we have a new Job.
    save();
    emit jobAdded(_job);
}

void JobDetailsWidget::updateMatchingArchives(const QList<ArchivePtr> &archives)
{
    // Do we need to prompt the user to check the Job's ArchiveList tab?
    if(!archives.isEmpty())
    {
        _ui->infoLabel->messageNormal(
            tr("Found %1 unassigned archives matching this Job description. Go "
               "to Archives tab below to review.")
                .arg(archives.count()));
        _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(
                                          _ui->archiveListTab),
                                      true);
    }
    else
    {
        _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(
                                          _ui->archiveListTab),
                                      false);
    }

    // Update the job's Archive list.
    _job->setArchives(archives);

    // Show the Archives in the UI.
    _ui->archiveListWidget->setArchives(_job->archives());
    _ui->tabWidget->setTabText(
        _ui->tabWidget->indexOf(_ui->archiveListTab),
        tr("Archives (%1)").arg(_job->archives().count()));
}

void JobDetailsWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateKeyboardShortcutInfo();
        if(_job)
        {
            if(_job->objectKey().isEmpty())
                canSaveNew();
            else
                updateDetails();
        }
    }
    QWidget::changeEvent(event);
}

void JobDetailsWidget::updateDetails()
{
    if(!_job)
        return;

    // Job is not ready for saving yet.
    _saveEnabled = false;

    // Show name and urls.
    _ui->jobNameLabel->setText(_job->name());
    _ui->jobTreeWidget->blockSignals(true);
    _ui->jobTreeWidget->setSelectedUrls(_job->urls());
    _ui->jobTreeWidget->blockSignals(false);

    // Show other Job data.
    _ui->jobTreeWidget->setSettingShowHidden(_job->settingShowHidden());
    _ui->jobTreeWidget->setSettingShowSystem(_job->settingShowSystem());
    _ui->jobTreeWidget->setSettingHideSymlinks(_job->settingHideSymlinks());
    _ui->scheduleComboBox->setCurrentIndex(
        static_cast<int>(_job->optionScheduledEnabled()));
    _ui->preservePathsCheckBox->setChecked(_job->optionPreservePaths());
    _ui->traverseMountCheckBox->setChecked(_job->optionTraverseMount());
    _ui->followSymLinksCheckBox->setChecked(_job->optionFollowSymLinks());
    _ui->skipNoDumpCheckBox->setChecked(_job->optionSkipNoDump());
    _ui->skipFilesSizeSpinBox->setValue(_job->optionSkipFilesSize());
    _ui->skipFilesCheckBox->setChecked(_job->optionSkipFiles());
    _ui->skipFilesLineEdit->setText(_job->optionSkipFilesPatterns());

    // Show any associated Archives.
    _ui->archiveListWidget->setArchives(_job->archives());
    _ui->tabWidget->setTabEnabled(_ui->tabWidget->indexOf(_ui->archiveListTab),
                                  _job->archives().count());
    _ui->tabWidget->setTabText(
        _ui->tabWidget->indexOf(_ui->archiveListTab),
        tr("Archives (%1)").arg(_job->archives().count()));

    // Check that the URLs are valid, otherwise show an error message.
    verifyJob();

    // Job is ready for saving.
    _saveEnabled = true;

    // Needed when morphing a job from the Backup tab to Jobs tab.  There's
    // probably a better way of doing this.
    canSaveNew();
}

void JobDetailsWidget::restoreButtonClicked()
{
    // Bail if it's not relevant.
    if(!_job || _job->archives().isEmpty())
        return;

    // Get Archive to restore.
    ArchivePtr archive = _job->archives().first();

    // Launch the RestoreDialog.
    RestoreDialog *restoreDialog = new RestoreDialog(this, archive);
    connect(restoreDialog, &RestoreDialog::accepted, [this, restoreDialog] {
        emit restoreJobArchive(restoreDialog->archive(),
                               restoreDialog->getOptions());
    });
    restoreDialog->show();
}

void JobDetailsWidget::backupButtonClicked()
{
    if(_job)
        emit backupJob(_job);
}

bool JobDetailsWidget::canSaveNew()
{
    QString name = _ui->jobNameLineEdit->text();

    // Clear info label.
    _ui->infoLabel->clear();

    // Bail if this isn't applicable.
    if(!_job->objectKey().isEmpty() || name.isEmpty())
        return false;

    // Check that we don't have any leading or trailing whitespace.
    if(name.simplified() != name)
    {
        _ui->infoLabel->messageError(
            tr("Job name cannot contain a leading or trailing whitespace."));
        return false;
    }

    // Create a new Job.
    JobPtr newJob(new Job);
    newJob->setName(name);

    // Bail if it doesn't have a unique name.
    if(newJob->doesKeyExist(newJob->name()))
    {
        _ui->infoLabel->messageError(
            tr("Job name must be unique amongst existing Jobs."));
        return false;
    }

    // Start looking for matching archives.
    emit findMatchingArchives(newJob->archivePrefix());

    // Bail if we don't have any selected urls.
    if(_ui->jobTreeWidget->getSelectedUrls().isEmpty())
    {
        _ui->infoLabel->messageError(tr("No backup paths selected."));
        return false;
    }

    // We're ok.
    return true;
}

void JobDetailsWidget::showArchiveListMenu()
{
    // Bail if not applicable.
    if(_ui->archiveListWidget->selectedItems().isEmpty())
        return;

    // Construct menu.
    _archiveListMenu->clear();
    if(_ui->archiveListWidget->selectedItems().count() == 1)
    {
        _archiveListMenu->addAction(_ui->actionInspect);
        _archiveListMenu->addAction(_ui->actionRestore);
    }
    _archiveListMenu->addAction(_ui->actionDelete);

    // Show menu.
    _archiveListMenu->popup(QCursor::pos());
}

void JobDetailsWidget::fsEventReceived()
{
    // Start (or re-start) a 250ms timer.  If this isn't restarted before it
    // triggers, it will signal verifyJob() to run.  If we have continuous
    // filesystem events (within 250ms of each other), verifyJob() will not
    // be called until they've finished.
    _fsEventUpdate->start(250);
}

void JobDetailsWidget::showJobPathsWarn()
{
    // Bail if not applicable.
    if(_job->urls().isEmpty())
        return;

    // Warn user about missing paths.
    QMessageBox *msg = new QMessageBox(this);
    msg->setAttribute(Qt::WA_DeleteOnClose, true);
    msg->setText(tr("Previously selected backup paths for this Job are not"
                    " accessible anymore and thus backups may be incomplete."
                    " Mount missing drives or make a new selection. Press Show"
                    " details to list all backup paths for Job %1:")
                     .arg(_job->name()));
    QStringList urls;
    for(const QUrl &url : _job->urls())
        urls << url.toLocalFile();
    msg->setDetailedText(urls.join('\n'));
    msg->show();
}

void JobDetailsWidget::verifyJob()
{
    // Bail if not applicable.
    if(_job->objectKey().isEmpty())
        return;

    // Show urls.
    _ui->jobTreeWidget->blockSignals(true);
    _ui->jobTreeWidget->setSelectedUrls(_job->urls());
    _ui->jobTreeWidget->blockSignals(false);

    // Check urls and show a warning (if applicable).
    bool validUrls = _job->validateUrls();
    if(validUrls)
        _ui->infoLabel->clear();
    else
    {
        if(_job->urls().isEmpty())
        {
            _ui->infoLabel->messageError(
                tr("This Job has no backup paths selected. "
                   "Please make a selection."));
        }
        else
        {
            _ui->infoLabel->messageError(
                tr("Previously selected backup paths are not"
                   " accessible. Click here for details."));
        }
    }
}

void JobDetailsWidget::updateKeyboardShortcutInfo()
{
    _ui->hideButton->setToolTip(_ui->hideButton->toolTip().arg(
        QKeySequence(Qt::Key_Escape).toString(QKeySequence::NativeText)));
}

void JobDetailsWidget::updateIEC()
{
    _ui->archiveListWidget->updateIEC();
}

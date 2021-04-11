#include "jobstabwidget.h"

WARNINGS_DISABLE
#include <QAction>
#include <QComboBox>
#include <QCursor>
#include <QDir>
#include <QEvent>
#include <QFrame>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QVariant>
#include <QWidget>
#include <Qt>

#include "ui_jobstabwidget.h"
WARNINGS_ENABLE

#include "TSettings.h"

#include "backuptask.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "widgets/joblistwidget.h"
#include "widgets/jobwidget.h"

const char *const DEFAULT_JOBS[] = {"Desktop", "Documents", "Pictures",
                                    "Movies",  "Videos",    "Music",
                                    "Work"};

JobsTabWidget::JobsTabWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::JobsTabWidget)
{
    // Ui initialization
    _ui->setupUi(this);

    _jobListMenu = new QMenu(_ui->jobListWidget);
    _ui->jobListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->jobDetailsWidget->hide();
    _ui->jobsFilterFrame->hide();

    // "Default jobs" handling
    connect(_ui->sureButton, &QPushButton::clicked, this,
            &JobsTabWidget::addDefaultJobs);
    connect(_ui->dismissButton, &QPushButton::clicked, [this]() {
        TSettings settings;
        settings.setValue("app/default_jobs_dismissed", true);
        _ui->defaultJobs->hide();
        _ui->addJobButton->show();
    });

    // Messages between widgets on this tab
    connect(_ui->addJobButton, &QToolButton::clicked, this,
            &JobsTabWidget::addJobClicked);
    connect(_ui->actionAddJob, &QAction::triggered, this,
            &JobsTabWidget::addJobClicked);

    // Connections from the JobDetailsWidget
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::collapse, this,
            &JobsTabWidget::hideJobDetails);
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::jobAdded, this,
            &JobsTabWidget::displayJobDetails);
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::enableSave,
            _ui->addJobButton, &QToolButton::setEnabled);

    // Connections from the JobDetailsWidget that get forwarded on
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::jobAdded, this,
            &JobsTabWidget::jobAdded);
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::inspectJobArchive, this,
            &JobsTabWidget::displayInspectArchive);
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::restoreJobArchive, this,
            &JobsTabWidget::restoreArchive);
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::deleteJobArchives, this,
            &JobsTabWidget::deleteArchives);
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::backupJob, this,
            &JobsTabWidget::backupJob);
    connect(_ui->jobDetailsWidget, &JobDetailsWidget::findMatchingArchives,
            this, &JobsTabWidget::findMatchingArchives);

    // Connections to the JobDetailsWidget
    connect(this, &JobsTabWidget::matchingArchives, _ui->jobDetailsWidget,
            &JobDetailsWidget::updateMatchingArchives);

    // Connections from the JobListWidget
    connect(_ui->jobListWidget, &JobListWidget::displayJobDetails, this,
            &JobsTabWidget::displayJobDetails);
    connect(_ui->jobListWidget, &JobListWidget::backupJob, this,
            &JobsTabWidget::backupJob);
    connect(_ui->jobListWidget, &JobListWidget::restoreArchive, this,
            &JobsTabWidget::restoreArchive);
    connect(_ui->jobListWidget, &JobListWidget::deleteJob, this,
            &JobsTabWidget::deleteJob);
    connect(_ui->jobListWidget, &JobListWidget::customContextMenuRequested,
            this, &JobsTabWidget::showJobsListMenu);

    // Connections to the JobListWidget
    connect(this, &JobsTabWidget::backupSelectedItems, _ui->jobListWidget,
            &JobListWidget::backupSelectedItems);
    connect(this, &JobsTabWidget::deleteSelectedItem, _ui->jobListWidget,
            &JobListWidget::deleteSelectedItem);
    connect(this, &JobsTabWidget::restoreSelectedItem, _ui->jobListWidget,
            &JobListWidget::restoreSelectedItem);
    connect(this, &JobsTabWidget::inspectSelectedItem, _ui->jobListWidget,
            &JobListWidget::inspectSelectedItem);

    // Jobs filter
    _ui->jobsFilterButton->setDefaultAction(_ui->actionFilterJobs);
    connect(_ui->actionFilterJobs, &QAction::triggered, [this]() {
        _ui->jobsFilterFrame->setVisible(!_ui->jobsFilterFrame->isVisible());
        if(_ui->jobsFilter->isVisible())
            _ui->jobsFilter->setFocus();
        else
            _ui->jobsFilter->clearEditText();
    });
    connect(_ui->jobsFilter, &QComboBox::editTextChanged, _ui->jobListWidget,
            &JobListWidget::setFilter);
    connect(_ui->jobsFilter,
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
            [this]() { _ui->jobListWidget->setFocus(); });

    // Update filter results
    connect(_ui->jobListWidget, &JobListWidget::countChanged,
            [this](int total, int visible) {
                _ui->jobsCountLabel->setText(
                    tr("Jobs (%1/%2)").arg(visible).arg(total));
            });

    // Right-click context menu
    _ui->jobListWidget->addAction(_ui->actionJobBackup);
    _ui->jobListWidget->addAction(_ui->actionJobDelete);
    _ui->jobListWidget->addAction(_ui->actionJobInspect);
    _ui->jobListWidget->addAction(_ui->actionJobRestore);
    _ui->jobListWidget->addAction(_ui->actionFilterJobs);
    _ui->jobListWidget->addAction(_ui->actionJobBackup);
    _ui->jobListWidget->addAction(_ui->actionAddJob);

    // Connections to the JobListWidget
    connect(this, &JobsTabWidget::jobList, _ui->jobListWidget,
            &JobListWidget::setJobs);
    connect(this, &JobsTabWidget::jobAdded, _ui->jobListWidget,
            &JobListWidget::addJob);
    connect(this, &JobsTabWidget::jobInspectByRef, _ui->jobListWidget,
            &JobListWidget::inspectJobByRef);

    connect(_ui->actionJobBackup, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::backupSelectedItems);
    connect(_ui->actionJobDelete, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::deleteSelectedItem);
    connect(_ui->actionJobRestore, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::restoreSelectedItem);
    connect(_ui->actionJobInspect, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::inspectSelectedItem);

    //  addJobButton
    QMenu *addJobMenu = new QMenu(_ui->addJobButton);
    addJobMenu->addAction(_ui->actionBackupAllJobs);
    connect(_ui->actionBackupAllJobs, &QAction::triggered, _ui->jobListWidget,
            &JobListWidget::backupAllJobs);
    _ui->addJobButton->setMenu(addJobMenu);

    loadSettings();
    updateKeyboardShortcutInfo();
    updateStatus();
}

JobsTabWidget::~JobsTabWidget()
{
    delete _ui;
}

void JobsTabWidget::loadSettings()
{
    TSettings settings;

    // Display the "add default jobs?" prompt (or not).
    if(settings.value("app/default_jobs_dismissed", false).toBool())
    {
        _ui->defaultJobs->hide();
        _ui->addJobButton->show();
    }
    else
    {
        _ui->defaultJobs->show();
        _ui->addJobButton->hide();
    }
}

void JobsTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateKeyboardShortcutInfo();
        updateStatus();
    }
    QWidget::changeEvent(event);
}

void JobsTabWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        // Close the JobDetailsWidget, or...
        if(_ui->jobDetailsWidget->isVisible())
        {
            hideJobDetails();
            return;
        }
        // ... cancel the Jobs filter.
        if(_ui->jobsFilter->isVisible())
        {
            if(_ui->jobsFilter->currentText().isEmpty())
            {
                _ui->actionFilterJobs->trigger();
            }
            else
            {
                _ui->jobsFilter->clearEditText();
                _ui->jobsFilter->setFocus();
            }
            return;
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void JobsTabWidget::updateKeyboardShortcutInfo()
{
    _ui->addJobButton->setToolTip(_ui->addJobButton->toolTip().arg(
        _ui->actionAddJob->shortcut().toString(QKeySequence::NativeText)));

    _ui->actionFilterJobs->setToolTip(_ui->actionFilterJobs->toolTip().arg(
        _ui->actionFilterJobs->shortcut().toString(QKeySequence::NativeText)));
    _ui->jobsFilter->setToolTip(_ui->jobsFilter->toolTip().arg(
        _ui->actionFilterJobs->shortcut().toString(QKeySequence::NativeText)));
}

void JobsTabWidget::updateStatus()
{
    // Ensure that the text on the "Save / Add job" button matches its state.
    if(_ui->addJobButton->property("save").toBool())
        _ui->addJobButton->setText(tr("Save"));
    else
        _ui->addJobButton->setText(tr("Add job"));
}

void JobsTabWidget::addDefaultJobs()
{
    TSettings settings;

    // Add a job for every directory in DEFAULT_JOBS that exists.
    for(const char *path : DEFAULT_JOBS)
    {
        QDir dir(QDir::home());
        if(dir.cd(path))
        {
            JobPtr job(new Job());
            job->setName(dir.dirName());
            QList<QUrl> urls;
            urls << QUrl::fromUserInput(dir.canonicalPath());
            job->setUrls(urls);
            job->save();
            _ui->jobDetailsWidget->jobAdded(job);
        }
    }

    // We don't need to see the "add default jobs?" prompt again.
    settings.setValue("app/default_jobs_dismissed", true);
    _ui->defaultJobs->hide();
    _ui->addJobButton->show();
}

void JobsTabWidget::addJobClicked()
{
    // Bail (if applicable).
    if(!_ui->addJobButton->isEnabled())
        return;

    // Act differently depending on the button state.
    if(_ui->addJobButton->property("save").toBool())
    {
        // Do stuff if the button says "Save".
        _ui->jobDetailsWidget->saveNew();
        _ui->addJobButton->setText(tr("Add job"));
        _ui->addJobButton->setProperty("save", false);
        _ui->addJobButton->setEnabled(true);
    }
    else
    {
        // Do stuff if the button says "Add Job".
        JobPtr job(new Job());
        displayJobDetails(job);
        _ui->addJobButton->setEnabled(false);
        _ui->addJobButton->setText(tr("Save"));
        _ui->addJobButton->setProperty("save", true);
    }
}

void JobsTabWidget::hideJobDetails()
{
    // Hide the details widget.
    _ui->jobDetailsWidget->hide();

    // Change the text on the "Save / Add job" button.
    if(_ui->addJobButton->property("save").toBool())
    {
        _ui->addJobButton->setText(tr("Add job"));
        _ui->addJobButton->setProperty("save", false);
        _ui->addJobButton->setEnabled(true);
    }
}

void JobsTabWidget::createNewJob(const QList<QUrl> &urls, const QString &name)
{
    // Create a new Job.
    JobPtr job(new Job());
    job->setUrls(urls);
    job->setName(name);

    // Disply details about the Job.
    displayJobDetails(job);

    // Change the text on the "Save / Add job" button.
    _ui->addJobButton->setEnabled(true);
    _ui->addJobButton->setText(tr("Save"));
    _ui->addJobButton->setProperty("save", true);
}

void JobsTabWidget::displayJobDetails(JobPtr job)
{
    // Display details about the Job.
    hideJobDetails();
    _ui->jobDetailsWidget->setJob(job);
    _ui->jobDetailsWidget->show();

    // Make sure that the app has actually expanded the JobDetailsWidget and
    // recalculated the space available for JobListWidget, before trying to
    // select the job in the list.
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);

    // Select the job in the list.
    _ui->jobListWidget->selectJob(job);
}

void JobsTabWidget::showJobsListMenu()
{
    // Construct menu.
    _jobListMenu->clear();
    if(!_ui->jobListWidget->selectedItems().isEmpty())
    {
        _jobListMenu->addAction(_ui->actionJobBackup);
        if(_ui->jobListWidget->selectedItems().count() == 1)
        {
            _jobListMenu->addAction(_ui->actionJobInspect);
            _jobListMenu->addAction(_ui->actionJobRestore);
            _jobListMenu->addAction(_ui->actionJobDelete);
        }
    }
    else if(_ui->jobListWidget->count() != 0)
    {
        _jobListMenu->addAction(_ui->actionBackupAllJobs);
    }

    // Display menu.
    _jobListMenu->popup(QCursor::pos());
}

void JobsTabWidget::backupJob(JobPtr job)
{
    // Bail (if applicable).
    if(!job)
        return;

    // Check for valid URLs.
    if(!job->validateUrls())
    {
        if(job->urls().isEmpty())
        {
            // Warn & bail if there's nothing selected.
            QMessageBox::warning(this, tr("Job error"),
                                 tr("Job %1 has no backup paths selected. "
                                    "Nothing to back up.")
                                     .arg(job->name()));
            return;
        }
        else
        {
            // Warn & bail if any paths are no longer valid.
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, tr("Job warning"),
                tr("Some backup paths for Job %1 are not"
                   " accessible anymore and thus backup may"
                   " be incomplete."
                   " Proceed with backup?")
                    .arg(job->name()));
            if(confirm != QMessageBox::Yes)
                return;
        }
    }
    emit backupNow(BackupTaskData::createBackupTaskFromJob(job));
}

void JobsTabWidget::updateIEC()
{
    _ui->jobDetailsWidget->updateIEC();
    _ui->jobListWidget->updateIEC();
}

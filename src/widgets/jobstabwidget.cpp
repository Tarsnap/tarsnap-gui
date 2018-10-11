#include "jobstabwidget.h"

#include "persistentmodel/job.h"
#include "utils.h"

#include <QMenu>
#include <QWidget>

#include <TSettings.h>

JobsTabWidget::JobsTabWidget(QWidget *parent) : QWidget(parent)
{
    // Ui initialization
    _ui.setupUi(this);
    _ui.jobDetailsWidget->hide();

    // "Default jobs" handling
    connect(_ui.sureButton, &QPushButton::clicked, this,
            &JobsTabWidget::addDefaultJobs);
    connect(_ui.dismissButton, &QPushButton::clicked, [&]() {
        TSettings settings;
        settings.setValue("app/default_jobs_dismissed", true);
        _ui.defaultJobs->hide();
        _ui.addJobButton->show();
    });

    // Messages between widgets on this tab
    connect(_ui.addJobButton, &QToolButton::clicked, this,
            &JobsTabWidget::addJobClicked);
    connect(_ui.actionAddJob, &QAction::triggered, this,
            &JobsTabWidget::addJobClicked);

    // Connections from the JobDetailsWidget
    connect(_ui.jobDetailsWidget, &JobWidget::collapse, this,
            &JobsTabWidget::hideJobDetails);
    connect(_ui.jobDetailsWidget, &JobWidget::jobAdded, this,
            &JobsTabWidget::displayJobDetails);
    connect(_ui.jobDetailsWidget, &JobWidget::enableSave, _ui.addJobButton,
            &QToolButton::setEnabled);

    // Connections from the JobDetailsWidget that get forwarded on
    connect(_ui.jobDetailsWidget, &JobWidget::jobAdded, this,
            &JobsTabWidget::jobAdded);
    connect(_ui.jobDetailsWidget, &JobWidget::inspectJobArchive, this,
            &JobsTabWidget::displayInspectArchive);
    connect(_ui.jobDetailsWidget, &JobWidget::restoreJobArchive, this,
            &JobsTabWidget::restoreArchive);
    connect(_ui.jobDetailsWidget, &JobWidget::deleteJobArchives, this,
            &JobsTabWidget::deleteArchives);
    connect(_ui.jobDetailsWidget, &JobWidget::backupJob, this,
            &JobsTabWidget::backupJob);
    connect(_ui.jobDetailsWidget, &JobWidget::findMatchingArchives, this,
            &JobsTabWidget::findMatchingArchives);

    // Connections to the JobDetailsWidget
    connect(this, &JobsTabWidget::matchingArchives, _ui.jobDetailsWidget,
            &JobWidget::updateMatchingArchives);

    loadSettings();
    updateUi();
}

void JobsTabWidget::loadSettings()
{
    TSettings settings;

    if(settings.value("app/default_jobs_dismissed", false).toBool())
    {
        _ui.defaultJobs->hide();
        _ui.addJobButton->show();
    }
    else
    {
        _ui.defaultJobs->show();
        _ui.addJobButton->hide();
    }
}

void JobsTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui.retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void JobsTabWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(_ui.jobDetailsWidget->isVisible())
        {
            hideJobDetails();
            return;
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void JobsTabWidget::updateUi()
{
    _ui.addJobButton->setToolTip(_ui.addJobButton->toolTip().arg(
        _ui.actionAddJob->shortcut().toString(QKeySequence::NativeText)));

    if(_ui.addJobButton->property("save").toBool())
        _ui.addJobButton->setText(tr("Save"));
    else
        _ui.addJobButton->setText(tr("Add job"));
}

void JobsTabWidget::addDefaultJobs()
{
    TSettings settings;
    for(const QString &path : DEFAULT_JOBS)
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
            _ui.jobDetailsWidget->jobAdded(job);
        }
    }
    settings.setValue("app/default_jobs_dismissed", true);
    _ui.defaultJobs->hide();
    _ui.addJobButton->show();
}

void JobsTabWidget::addJobClicked()
{
    if(!_ui.addJobButton->isEnabled())
        return;

    if(_ui.addJobButton->property("save").toBool())
    {
        _ui.jobDetailsWidget->saveNew();
        _ui.addJobButton->setText(tr("Add job"));
        _ui.addJobButton->setProperty("save", false);
        _ui.addJobButton->setEnabled(true);
    }
    else
    {
        JobPtr job(new Job());
        displayJobDetails(job);
        _ui.addJobButton->setEnabled(false);
        _ui.addJobButton->setText(tr("Save"));
        _ui.addJobButton->setProperty("save", true);
    }
}

void JobsTabWidget::hideJobDetails()
{
    _ui.jobDetailsWidget->hide();
    if(_ui.addJobButton->property("save").toBool())
    {
        _ui.addJobButton->setText(tr("Add job"));
        _ui.addJobButton->setProperty("save", false);
        _ui.addJobButton->setEnabled(true);
    }
}

void JobsTabWidget::createNewJob(QList<QUrl> urls, QString name)
{
    JobPtr job(new Job());
    job->setUrls(urls);
    job->setName(name);
    displayJobDetails(job);
    _ui.addJobButton->setEnabled(true);
    _ui.addJobButton->setText(tr("Save"));
    _ui.addJobButton->setProperty("save", true);
}

void JobsTabWidget::displayJobDetails(JobPtr job)
{
    hideJobDetails();
    _ui.jobDetailsWidget->setJob(job);
    _ui.jobDetailsWidget->show();
}

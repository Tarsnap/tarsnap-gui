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
            //_ui.jobDetailsWidget->jobAdded(job);
            emit temp_jobDetailsWidget_jobAdded(job);
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
        //_ui.jobDetailsWidget->saveNew();
        emit temp_jobDetailsWidget_saveNew();
        _ui.addJobButton->setText(tr("Add job"));
        _ui.addJobButton->setProperty("save", false);
        _ui.addJobButton->setEnabled(true);
    }
    else
    {
        JobPtr job(new Job());
        // displayJobDetails(job);
        emit temp_displayJobDetails(job);
        _ui.addJobButton->setEnabled(false);
        _ui.addJobButton->setText(tr("Save"));
        _ui.addJobButton->setProperty("save", true);
    }
}

void JobsTabWidget::hideJobDetails()
{
    //_ui.jobDetailsWidget->hide();
    if(_ui.addJobButton->property("save").toBool())
    {
        _ui.addJobButton->setText(tr("Add job"));
        _ui.addJobButton->setProperty("save", false);
        _ui.addJobButton->setEnabled(true);
    }
}

void JobsTabWidget::createNewJob(QList<QUrl> urls, QString name)
{
    (void)urls;
    (void)name;
    // JobPtr job(new Job());
    // job->setUrls(urls);
    // job->setName(name);
    // displayJobDetails(job);
    _ui.addJobButton->setEnabled(true);
    _ui.addJobButton->setText(tr("Save"));
    _ui.addJobButton->setProperty("save", true);
}

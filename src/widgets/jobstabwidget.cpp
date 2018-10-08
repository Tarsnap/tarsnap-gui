#include "jobstabwidget.h"

#include "persistentmodel/job.h"
#include "utils.h"

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
        //_ui.addJobButton->show();
        emit temp_addJobButton_show(true);
    });

    loadSettings();
}

void JobsTabWidget::loadSettings()
{
    TSettings settings;

    if(settings.value("app/default_jobs_dismissed", false).toBool())
    {
        _ui.defaultJobs->hide();
        //_ui.addJobButton->show();
    }
    else
    {
        _ui.defaultJobs->show();
        //_ui.addJobButton->hide();
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
            temp_jobDetailsWidget_jobAdded(job);
        }
    }
    settings.setValue("app/default_jobs_dismissed", true);
    _ui.defaultJobs->hide();
    //_ui.addJobButton->show();
    emit temp_addJobButton_show(true);
}

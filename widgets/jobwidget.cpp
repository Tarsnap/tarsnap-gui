#include "jobwidget.h"
#include "ui_jobwidget.h"
#include "debug.h"

JobWidget::JobWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::JobWidget)
{
    _ui->setupUi(this);
    connect(_ui->listArchivesButton, &QPushButton::clicked,
            [=](){
                _ui->stackedWidget->setCurrentWidget(_ui->jobRestorePage);
            });
    connect(_ui->restoreBackButton, &QPushButton::clicked,
            [=](){
                _ui->stackedWidget->setCurrentWidget(_ui->jobDetailPage);
            });
    connect(_ui->optionsBackButton, &QPushButton::clicked,
            [=](){
                _ui->stackedWidget->setCurrentWidget(_ui->jobDetailPage);
            });
    connect(_ui->optionsButton, &QPushButton::clicked,
            [=](){
                _ui->stackedWidget->setCurrentWidget(_ui->jobOptionsPage);
            });
    connect(_ui->cancelButton, SIGNAL(clicked()), this, SIGNAL(cancel()));
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
    if(!_job.isNull() && !_job->objectKey().isEmpty())
        disconnect(_ui->detailTreeWidget, SIGNAL(selectionChanged()), this, SLOT(save()));
    _job = job;
    updateDetails();
}

void JobWidget::save()
{
    DEBUG << "SAVE";
    // save current or new job details
    if(_job->objectKey().isEmpty())
    {
        _job->setName(_ui->jobNameLineEdit->text());
        _job->setUrls(_ui->newJobTreeWidget->getSelectedUrls());
        emit jobAdded(_job);
    }
    else
    {
        _job->setUrls(_ui->detailTreeWidget->getSelectedUrls());
    }
    _job->save();
}

void JobWidget::updateDetails()
{
    if(_job->objectKey().isEmpty())
    {
        _ui->stackedWidget->setCurrentWidget(_ui->jobNewPage);
        _ui->jobNameLineEdit->setFocus();
    }
    else
    {
        _ui->stackedWidget->setCurrentWidget(_ui->jobDetailPage);
        _ui->jobNameLabel->setText(_job->name());
        _ui->detailTreeWidget->reset();
        _ui->detailTreeWidget->setSelectedUrls(_job->urls());
        connect(_ui->detailTreeWidget, SIGNAL(selectionChanged()), this, SLOT(save()));
    }
}


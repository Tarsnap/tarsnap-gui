#include "jobwidget.h"
#include "ui_jobwidget.h"

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
    _job = job;
    updateDetails();
}

void JobWidget::save()
{
    // save current or new job details
}

void JobWidget::updateDetails()
{
    if(_job->uuid().isNull())
    {
        _ui->stackedWidget->setCurrentWidget(_ui->jobNewPage);
//        _ui->jobNameLineEdit->clear();
    }
    else
    {
        _ui->stackedWidget->setCurrentWidget(_ui->jobDetailPage);
        _ui->jobNameLabel->setText(_job->name());
    }
}


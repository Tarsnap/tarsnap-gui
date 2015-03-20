#include "jobwidget.h"
#include "ui_jobwidget.h"

JobWidget::JobWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::JobWidget)
{
    _ui->setupUi(this);
    connect(_ui->browseButton, &QPushButton::clicked,
            [=](){
                _ui->stackedWidget->setCurrentWidget(_ui->restoreView);
            });
    connect(_ui->backButton, &QPushButton::clicked,
            [=](){
                _ui->stackedWidget->setCurrentWidget(_ui->treeView);
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
    updateDetails();
}

void JobWidget::updateDetails()
{
    _ui->jobNameLabel->setText(_job->name);
}


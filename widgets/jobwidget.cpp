#include "jobwidget.h"
#include "ui_jobwidget.h"
#include "restoredialog.h"
#include "debug.h"

JobWidget::JobWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::JobWidget)
{
    _ui->setupUi(this);
    _ui->restoreListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

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
    connect(_ui->restoreLatestArchiveButton, SIGNAL(clicked()), this, SLOT(restoreLatestArchive()));
    connect(_ui->restoreListWidget, SIGNAL(inspectArchive(ArchivePtr)), this, SIGNAL(inspectJobArchive(ArchivePtr)));
    connect(_ui->restoreListWidget, SIGNAL(restoreArchive(ArchivePtr,ArchiveRestoreOptions)), this, SIGNAL(restoreJobArchive(ArchivePtr,ArchiveRestoreOptions)));
    connect(_ui->restoreListWidget, SIGNAL(deleteArchives(QList<ArchivePtr>)), this, SIGNAL(deleteJobArchives(QList<ArchivePtr>)));
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
    if(_job && !_job->objectKey().isEmpty())
    {
        disconnect(_ui->detailTreeWidget, SIGNAL(selectionChanged()), this, SLOT(save()));
        disconnect(_job.data(), SIGNAL(changed()), this, SLOT(updateDetails()));
    }

    _job = job;

    if(_job->objectKey().isEmpty())
    {
        _ui->stackedWidget->setCurrentWidget(_ui->jobNewPage);
        _ui->jobNameLineEdit->setFocus();
    }
    else
    {
        _ui->stackedWidget->setCurrentWidget(_ui->jobDetailPage);
        updateDetails();
        connect(_ui->detailTreeWidget, SIGNAL(selectionChanged()), this, SLOT(save()));
        connect(_job.data(), SIGNAL(changed()), this, SLOT(updateDetails()));
    }
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
    if(_job)
    {
        _ui->jobNameLabel->setText(_job->name());
        disconnect(_ui->detailTreeWidget, SIGNAL(selectionChanged()), this, SLOT(save()));
        _ui->detailTreeWidget->reset();
        _ui->detailTreeWidget->setSelectedUrls(_job->urls());
        connect(_ui->detailTreeWidget, SIGNAL(selectionChanged()), this, SLOT(save()));
        _ui->restoreListWidget->clear();
        _ui->restoreListWidget->addArchives(_job->archives());
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


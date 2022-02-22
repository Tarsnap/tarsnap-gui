#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QList>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUrl>

#include "ui_jobstabwidget.h"
#include "ui_jobwidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "TSettings.h"

#include "messages/backuptaskdataptr.h"
#include "messages/jobptr.h"

#include "backuptask.h"
#include "persistentmodel/job.h"
#include "persistentmodel/persistentstore.h"
#include "widgets/elidedclickablelabel.h"
#include "widgets/joblistwidget.h"
#include "widgets/jobstabwidget.h"
#include "widgets/jobwidget.h"

class TestJobsTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test JobDetailsWidget in isolation
    void jobDetailsWidget();
    // This one needs to be before other JobsTabWidget tests
    void defaultJobs();
    void createJob();
    void displayJobDetails();
    void jobListWidget();
};

void TestJobsTabWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }

    // Initialization normally done in init_shared.cpp's init_no_app()
    qRegisterMetaType<JobPtr>("JobPtr");
    qRegisterMetaType<BackupTaskDataPtr>("BackupTaskDataPtr");

    // Deal with PersistentStore
    PersistentStore::initializePersistentStore();
    int ok = global_store->init();
    QVERIFY(ok);
}

void TestJobsTabWidget::cleanupTestCase()
{
    PersistentStore::destroy();
    TSettings::destroy();

    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

void TestJobsTabWidget::jobDetailsWidget()
{
    JobDetailsWidget *jobDetailsWidget = new JobDetailsWidget();

    // Add a Job to the widget.
    JobPtr job(new Job());
    job->setUrls(QList<QUrl>() << QUrl("file://" TEST_DIR));
    job->setName("jobDetailsWidget1");
    jobDetailsWidget->setJob(job);

    // Save it.
    jobDetailsWidget->saveNew();

    delete jobDetailsWidget;
}

void TestJobsTabWidget::defaultJobs()
{
    JobsTabWidget     *jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget *ui            = jobstabwidget->_ui;
    QSignalSpy         sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));

    VISUAL_INIT(jobstabwidget);

    // We start off showing the default jobs prompt
    QVERIFY(ui->defaultJobs->isVisibleTo(jobstabwidget) == true);

    // Don't create the default jobs; no more prompt
    ui->dismissButton->clicked();
    QVERIFY(ui->defaultJobs->isVisibleTo(jobstabwidget) == false);
    QVERIFY(sig_jobAdded.count() == 0);
    VISUAL_WAIT;

    delete jobstabwidget;
}

void TestJobsTabWidget::createJob()
{
    JobsTabWidget        *jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget    *ui            = jobstabwidget->_ui;
    JobDetailsWidget     *jobWidget     = jobstabwidget->_ui->jobDetailsWidget;
    Ui::JobDetailsWidget *jobWidget_ui  = jobWidget->_ui;
    QSignalSpy            sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));

    VISUAL_INIT(jobstabwidget);

    // Start out without the button being enabled
    QVERIFY(ui->addJobButton->text() == QString("Add job"));
    QVERIFY(sig_jobAdded.count() == 0);
    VISUAL_WAIT;

    // Create a job
    jobstabwidget->createNewJob(QList<QUrl>() << QUrl("file://" TEST_DIR),
                                QString("test-job"));
    QVERIFY(ui->addJobButton->text() == QString("Save"));
    QVERIFY(ui->addJobButton->isEnabled());
    QVERIFY(jobWidget_ui->infoLabel->text() == "");
    VISUAL_WAIT;

    // Adds the job to the list; makes the button ready to add again
    jobstabwidget->addJobClicked();
    QVERIFY(ui->addJobButton->text() == QString("Add job"));
    QVERIFY(sig_jobAdded.count() == 1);
    VISUAL_WAIT;

    // Create a second job
    jobstabwidget->createNewJob(QList<QUrl>() << QUrl("file://" TEST_DIR),
                                QString("test-job-2"));
    QVERIFY(ui->addJobButton->text() == QString("Save"));
    QVERIFY(ui->addJobButton->isEnabled());
    QVERIFY(jobWidget_ui->infoLabel->text() == "");
    VISUAL_WAIT;

    // Adds the job to the list; makes the button ready to add again
    jobstabwidget->addJobClicked();
    QVERIFY(ui->addJobButton->text() == QString("Add job"));
    QVERIFY(sig_jobAdded.count() == 2);
    VISUAL_WAIT;

    delete jobstabwidget;
}

void TestJobsTabWidget::displayJobDetails()
{
    JobsTabWidget     *jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget *ui            = jobstabwidget->_ui;
    QSignalSpy         sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));
    QSignalSpy         sig_backupNow(jobstabwidget,
                                     SIGNAL(backupNow(BackupTaskDataPtr)));

    VISUAL_INIT(jobstabwidget);

    // Create a job
    jobstabwidget->createNewJob(QList<QUrl>() << QUrl("file://" TEST_DIR),
                                QString("test-job-display"));
    jobstabwidget->addJobClicked();
    JobPtr job = sig_jobAdded.takeFirst().at(0).value<JobPtr>();
    VISUAL_WAIT;

    // Don't show the job
    jobstabwidget->hideJobDetails();
    QVERIFY(ui->jobDetailsWidget->isVisibleTo(jobstabwidget) == false);
    VISUAL_WAIT;

    // Show the job
    jobstabwidget->displayJobDetails(job);
    QVERIFY(ui->jobDetailsWidget->isVisibleTo(jobstabwidget) == true);
    VISUAL_WAIT;

    // Create a new archive for the job
    QVERIFY(sig_backupNow.count() == 0);
    ui->jobDetailsWidget->backupButtonClicked();
    QVERIFY(sig_backupNow.count() == 1);
    VISUAL_WAIT;

    delete jobstabwidget;
}

void TestJobsTabWidget::jobListWidget()
{
    JobsTabWidget     *jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget *ui            = jobstabwidget->_ui;
    QSignalSpy         sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));
    QSignalSpy sig_deleteJob(jobstabwidget, SIGNAL(deleteJob(JobPtr, bool)));

    VISUAL_INIT(jobstabwidget);

    // Create a job
    jobstabwidget->createNewJob(QList<QUrl>() << QUrl("file://" TEST_DIR),
                                QString("test-job-joblistwidget"));
    jobstabwidget->addJobClicked();
    JobPtr job = sig_jobAdded.takeFirst().at(0).value<JobPtr>();
    QVERIFY(ui->jobListWidget->count() == 1);
    VISUAL_WAIT;

    // Don't show the job
    jobstabwidget->hideJobDetails();
    QVERIFY(ui->jobDetailsWidget->isVisibleTo(jobstabwidget) == false);
    VISUAL_WAIT;

    // Show the job with the joblistwidget
    jobstabwidget->jobInspectByRef("test-job-joblistwidget");
    QVERIFY(ui->jobDetailsWidget->isVisibleTo(jobstabwidget) == true);
    VISUAL_WAIT;

    delete jobstabwidget;
}

QTEST_MAIN(TestJobsTabWidget)
WARNINGS_DISABLE
#include "test-jobstabwidget.moc"
WARNINGS_ENABLE

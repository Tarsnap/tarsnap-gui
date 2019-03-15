#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include "ui_jobstabwidget.h"
#include "ui_jobwidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include <TSettings.h>

#include "jobstabwidget.h"
#include "jobwidget.h"

class TestJobsTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Test JobWidget in isolation
    void jobWidget();
    // This one needs to be before other JobsTabWidget tests
    void defaultJobs();
    void createJob();
    void displayJobDetails();
    void jobListWidget();
};

void TestJobsTabWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }

    // Deal with PersistentStore
    PersistentStore::initializePersistentStore();
    PersistentStore &store = PersistentStore::instance();
    int              ok    = store.init();
    QVERIFY(ok);
}

void TestJobsTabWidget::cleanupTestCase()
{
    PersistentStore::destroy();
    TSettings::destroy();
    // Wait to (probably?) allow something within QWidgetLineControl to
    // get freed.  (Randomly occurring memorty leak.)
    QTest::qWait(100);
}

void TestJobsTabWidget::jobWidget()
{
    JobWidget *jobWidget = new JobWidget();
    delete jobWidget;
}

void TestJobsTabWidget::defaultJobs()
{
    JobsTabWidget *    jobstabwidget = new JobsTabWidget();
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
    JobsTabWidget *    jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget *ui            = jobstabwidget->_ui;
    JobWidget *        jobWidget     = jobstabwidget->_ui->jobDetailsWidget;
    Ui::JobWidget *    jobWidget_ui  = jobWidget->_ui;
    QSignalSpy         sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));

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

    delete jobstabwidget;
}

void TestJobsTabWidget::displayJobDetails()
{
    JobsTabWidget *    jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget *ui            = jobstabwidget->_ui;
    QSignalSpy         sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));
    QSignalSpy sig_backupNow(jobstabwidget, SIGNAL(backupNow(BackupTaskPtr)));

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
    JobsTabWidget *    jobstabwidget = new JobsTabWidget();
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
#include "test-jobstabwidget.moc"

#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include <TSettings.h>

#include "jobstabwidget.h"

class TestJobsTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // This one needs to be first
    void defaultJobs();
    void createJob();
    void displayJobDetails();
};

void TestJobsTabWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }
}

void TestJobsTabWidget::cleanupTestCase()
{
    TSettings::destroy();
}

void TestJobsTabWidget::defaultJobs()
{
    JobsTabWidget *   jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget ui            = jobstabwidget->_ui;
    QSignalSpy        sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));

    VISUAL_INIT(jobstabwidget);
    jobstabwidget->show();

    // We start off showing the default jobs prompt
    QVERIFY(ui.defaultJobs->isVisible() == true);

    // Don't create the default jobs; no more prompt
    ui.dismissButton->clicked();
    QVERIFY(ui.defaultJobs->isVisible() == false);
    QVERIFY(sig_jobAdded.count() == 0);
    VISUAL_WAIT;

    delete jobstabwidget;
}

void TestJobsTabWidget::createJob()
{
    JobsTabWidget *   jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget ui            = jobstabwidget->_ui;
    QSignalSpy        sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));

    VISUAL_INIT(jobstabwidget);

    // Start out without the button being enabled
    QVERIFY(ui.addJobButton->text() == QString("Add job"));
    QVERIFY(sig_jobAdded.count() == 0);
    VISUAL_WAIT;

    // Create a job
    jobstabwidget->createNewJob(QList<QUrl>() << QUrl("file://" TEST_DIR),
                                QString("test-job"));
    QVERIFY(ui.addJobButton->text() == QString("Save"));
    VISUAL_WAIT;

    // Adds the job to the list; makes the button ready to add again
    jobstabwidget->addJobClicked();
    QVERIFY(ui.addJobButton->text() == QString("Add job"));
    QVERIFY(sig_jobAdded.count() == 1);
    VISUAL_WAIT;

    delete jobstabwidget;
}

void TestJobsTabWidget::displayJobDetails()
{
    JobsTabWidget *   jobstabwidget = new JobsTabWidget();
    Ui::JobsTabWidget ui            = jobstabwidget->_ui;
    QSignalSpy        sig_jobAdded(jobstabwidget, SIGNAL(jobAdded(JobPtr)));
    QSignalSpy        sig_jobBackup(jobstabwidget, SIGNAL(backupJob(JobPtr)));

    VISUAL_INIT(jobstabwidget);

    // Create a job
    jobstabwidget->createNewJob(QList<QUrl>() << QUrl("file://" TEST_DIR),
                                QString("test-job-display"));
    jobstabwidget->addJobClicked();
    JobPtr job = sig_jobAdded.takeFirst().at(0).value<JobPtr>();
    VISUAL_WAIT;

    // Don't show the job
    jobstabwidget->hideJobDetails();
    VISUAL_WAIT;

    // Show the job
    jobstabwidget->displayJobDetails(job);
    VISUAL_WAIT;

    // Create a new archive for the job
    QVERIFY(sig_jobBackup.count() == 0);
    ui.jobDetailsWidget->backupButtonClicked();
    QVERIFY(sig_jobBackup.count() == 1);
    VISUAL_WAIT;

    delete jobstabwidget;
}

QTEST_MAIN(TestJobsTabWidget)
#include "test-jobstabwidget.moc"

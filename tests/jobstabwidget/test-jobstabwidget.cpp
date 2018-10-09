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
    QSignalSpy        sig_jobAdded(jobstabwidget,
                            SIGNAL(temp_jobDetailsWidget_jobAdded(JobPtr)));

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

QTEST_MAIN(TestJobsTabWidget)
#include "test-jobstabwidget.moc"

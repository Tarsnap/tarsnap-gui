#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAction>
#include <QCoreApplication>
#include <QDialog>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QMetaType>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QTabWidget>
#include <QTest>
#include <QThreadPool>
#include <QUrl>
#include <QVector>

#include "ui_archivestabwidget.h"
#include "ui_helpwidget.h"
#include "ui_jobstabwidget.h"
#include "ui_mainwindow.h"
#include "ui_stoptasksdialog.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "TPopupPushButton.h"

#include "messages/archivefilestat.h"

#include "basetask.h"
#include "translator.h"
#include "widgets/archivestabwidget.h"
#include "widgets/backuptabwidget.h"
#include "widgets/helpwidget.h"
#include "widgets/jobstabwidget.h"
#include "widgets/mainwindow.h"
#include "widgets/statusbarwidget.h"
#include "widgets/stoptasksdialog.h"

#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "persistentmodel/persistentstore.h"
#include "widgets/archivewidget.h"
#include "widgets/filepickerdialog.h"
#include "widgets/joblistwidget.h"
#include "widgets/joblistwidgetitem.h"

#include "ConsoleLog.h"
#include "TSettings.h"

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void statusbarwidget();
    void about_window_menubar();
    void stoptasksdialog_nonquit();
    void quit_simple();
    void quit_tasks();
    void tab_navigation();
    void other_navigation();
};

void TestMainWindow::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    HANDLE_IGNORING_XDG_HOME;

    ConsoleLog::initializeConsoleLog();
    Translator::initializeTranslator();

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }

    // Initialization normally done in init_shared.cpp's init_no_app()
    qRegisterMetaType<QVector<FileStat>>("QVector<FileStat>");
    qRegisterMetaType<BaseTask *>("BaseTask *");

    // Deal with PersistentStore
    PersistentStore::initializePersistentStore();
    int ok = global_store->init();
    QVERIFY(ok);
}

void TestMainWindow::cleanupTestCase()
{
    PersistentStore::destroy();
    Translator::destroy();
    TSettings::destroy();
    ConsoleLog::destroy();

    // Wait up to 5 seconds for any running threads to stop.
    QThreadPool::globalInstance()->waitForDone(5000);
    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

void TestMainWindow::statusbarwidget()
{
    StatusBarWidget *sbw = new StatusBarWidget();

    VISUAL_INIT(sbw);

    delete sbw;
}

static QAction *get_menubar_about(QMenuBar *menubar)
{
    for(QAction *action : menubar->actions())
    {
        if(action->menu())
        {
            for(QAction *subaction : action->menu()->actions())
            {
                if(subaction->menuRole() == QAction::AboutRole)
                {
                    return subaction;
                }
            }
        }
    }
    return (nullptr);
}

void TestMainWindow::about_window_menubar()
{
    MainWindow     *mainwindow = new MainWindow();
    HelpWidget     *help       = mainwindow->_helpWidget;
    Ui::HelpWidget *ui         = help->_ui;

    VISUAL_INIT(mainwindow);

    // Test the "About Tarsnap" menubar item, if applicable.
    if(mainwindow->_menuBar != nullptr)
    {
        // find "About Tarsnap" menu item
        QAction *menuAction = get_menubar_about(mainwindow->_menuBar);
        QVERIFY(menuAction != nullptr);
        VISUAL_WAIT;

        // Becomes visible using the menu bar action
        menuAction->trigger();
        QVERIFY(help->_aboutWindow->isVisible() == true);
        QVERIFY(ui->aboutButton->isChecked() == true);
        VISUAL_WAIT;

        // Stay visible even when clicking the menu bar action again
        menuAction->trigger();
        QVERIFY(help->_aboutWindow->isVisible() == true);
        QVERIFY(ui->aboutButton->isChecked() == true);
        VISUAL_WAIT;

        // Becomes invisible by clicking the Help->About button
        ui->aboutButton->click();
        QVERIFY(help->_aboutWindow->isVisible() == false);
        QVERIFY(ui->aboutButton->isChecked() == false);
        VISUAL_WAIT;
    }

    delete mainwindow;
}

void TestMainWindow::stoptasksdialog_nonquit()
{
    MainWindow      *mainwindow = new MainWindow();
    StopTasksDialog *st         = mainwindow->_stopTasksDialog;

    VISUAL_INIT(mainwindow);

    // Prep with number of tasks.
    mainwindow->updateNumTasks(false, 1, 0);

    // Check initial visibility.
    QVERIFY(mainwindow->isVisible() == true);
    QVERIFY(st->isVisible() == false);

    // Trigger "stop tasks" dialog (without trying to quit).
    mainwindow->_ui->actionStopTasks->trigger();
    QVERIFY(st->isVisible() == true);
    VISUAL_WAIT;

    // No more tasks, so the StopTasksDialog should disappear.
    mainwindow->updateNumTasks(false, 0, 0);
    QVERIFY(mainwindow->isVisible() == true);
    QVERIFY(st->isVisible() == false);
    VISUAL_WAIT;

    delete mainwindow;
}

void TestMainWindow::quit_simple()
{
    MainWindow *mainwindow = new MainWindow();

    VISUAL_INIT(mainwindow);

    // We should be visible.
    QVERIFY(mainwindow->isVisible() == true);

    // Try to close the window; there's no tasks, so we should close.
    mainwindow->close();
    QVERIFY(mainwindow->isVisible() == false);

    delete mainwindow;
}

void TestMainWindow::quit_tasks()
{
    MainWindow *mainwindow = new MainWindow();

    VISUAL_INIT(mainwindow);

    // We should be visible.
    QVERIFY(mainwindow->isVisible() == true);

    // Prep with number of tasks.
    mainwindow->updateNumTasks(false, 1, 0);

    // Try to close the window.
    mainwindow->close();
    VISUAL_WAIT;

    // Cancel the quitting, and we should still be visible.
    mainwindow->_stopTasksDialog->close();
    VISUAL_WAIT;
    QVERIFY(mainwindow->isVisible() == true);

    // Try to close the window.
    mainwindow->close();
    VISUAL_WAIT;

    // Proceed with the quitting, and we should not be visible.
    mainwindow->_stopTasksDialog->_ui->stopAllButton->clicked();
    VISUAL_WAIT;
    QVERIFY(mainwindow->isVisible() == false);

    delete mainwindow;
}

void TestMainWindow::tab_navigation()
{
    MainWindow     *mainwindow = new MainWindow();
    Ui::MainWindow *ui         = mainwindow->_ui;

    VISUAL_INIT(mainwindow);

    // Start in the Backup tab
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->backupTab);

    // Switch between tabs
    // Unfortunately we can't test the Ctrl+X keyboard shortcuts, because
    // QTest::keyClick() doesn't work with platform=offscreen.
    mainwindow->displayTab(ui->archivesTab);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->archivesTab);
    VISUAL_WAIT;

    mainwindow->displayTab(ui->jobsTab);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->jobsTab);
    VISUAL_WAIT;

    mainwindow->displayTab(ui->settingsTab);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->settingsTab);
    VISUAL_WAIT;

    mainwindow->displayTab(ui->helpTab);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->helpTab);
    VISUAL_WAIT;

    mainwindow->displayTab(ui->backupTab);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->backupTab);
    VISUAL_WAIT;

    delete mainwindow;
}

void TestMainWindow::other_navigation()
{
    MainWindow        *mainwindow    = new MainWindow();
    Ui::MainWindow    *ui            = mainwindow->_ui;
    JobsTabWidget     *jobsTabWidget = mainwindow->_jobsTabWidget;
    Ui::JobsTabWidget *jui           = jobsTabWidget->_ui;

    QSignalSpy sig_taskRequest(mainwindow, SIGNAL(taskRequested(BaseTask *)));
    BaseTask  *task;

    BackupTabWidget *backupTabWidget = mainwindow->_backupTabWidget;

    ArchiveDetailsWidget *archiveDetailsWidget =
        mainwindow->_archivesTabWidget->_ui->archiveDetailsWidget;

    VISUAL_INIT(mainwindow);

#if defined(Q_OS_OSX)
    // HACK: Load directory that we'll want for creating a Job.  This is
    // slow to load on OSX (relative to the platform=offscreen test), so we
    // add an extra delay.
    jobsTabWidget->_ui->jobDetailsWidget->_ui->jobTreeWidget->_model
        .setRootPath(TEST_DIR);
    QTest::qWait(1000);
#endif

    QList<QUrl> testdir_urls({QUrl("file://" TEST_DIR)});

    // Switch to a different tab.
    mainwindow->displayTab(ui->helpTab);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->helpTab);
    VISUAL_WAIT;

    // Switch back to Backup tab
    mainwindow->browseForBackupItems();
    VISUAL_WAIT;
    backupTabWidget->_filePickerDialog->close();
    QTest::qWait(100);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->backupTab);
    VISUAL_WAIT;

    // Add a Job
    mainwindow->displayTab(ui->jobsTab);
    jobsTabWidget->createNewJob(testdir_urls, QString("test-job"));
    jobsTabWidget->addJobClicked();

    // Make sure that MainWindow has a job, then get a pointer to it.
    QVERIFY(jui->jobListWidget->count() == 1);
    QVERIFY(jui->jobListWidget->currentItem() != nullptr);
    JobPtr job =
        static_cast<JobListWidgetItem *>(jui->jobListWidget->currentItem())
            ->job();
    QVERIFY(job != nullptr);
    VISUAL_WAIT;

    // Add an Archive
    mainwindow->displayTab(ui->archivesTab);
    Archive   *actual_archive = new Archive();
    ArchivePtr archive(actual_archive);
    archive->setName("Job_test-job_archive1");
    archive->setContents("-rw-r--r-- 0 user group 1234 Jan 1 2019 myfile");
    mainwindow->addArchive(archive);
    archive->setJobRef("test-job");
    VISUAL_WAIT;

    // Link them
    job->setArchives(QList<ArchivePtr>() << archive);

    // Create a second job, this time via morphBackupIntoJob
    mainwindow->displayTab(ui->backupTab);
    VISUAL_WAIT;

    backupTabWidget->morphBackupIntoJob(testdir_urls, "test-job2");
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->jobsTab);
    VISUAL_WAIT;

    jobsTabWidget->addJobClicked();
    QVERIFY(jui->jobListWidget->count() == 2);
    VISUAL_WAIT;

    // Switch back and forth between the job and archive.
    mainwindow->displayInspectArchive(archive);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->archivesTab);
    VISUAL_WAIT;

    // Wait for archive parsing signal, and delete it.
    WAIT_SIG(sig_taskRequest);
    task = sig_taskRequest.takeFirst().at(0).value<BaseTask *>();
    delete task;

    mainwindow->displayJobDetails(job);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->jobsTab);
    VISUAL_WAIT;

    // Click on the Job name in the archive tab.
    mainwindow->displayInspectArchive(archive);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->archivesTab);
    VISUAL_WAIT;

    // Wait for archive parsing signal, and delete it.
    WAIT_SIG(sig_taskRequest);
    task = sig_taskRequest.takeFirst().at(0).value<BaseTask *>();
    delete task;

    archiveDetailsWidget->jobClicked("test-job");
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->jobsTab);
    VISUAL_WAIT;

    delete mainwindow;
}

QTEST_MAIN(TestMainWindow)
WARNINGS_DISABLE
#include "test-mainwindow.moc"
WARNINGS_ENABLE

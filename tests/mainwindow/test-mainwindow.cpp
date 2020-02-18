#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include "ui_archivestabwidget.h"
#include "ui_helpwidget.h"
#include "ui_jobstabwidget.h"
#include "ui_jobwidget.h"
#include "ui_mainwindow.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "helpwidget.h"
#include "jobstabwidget.h"
#include "mainwindow.h"

#include "archivewidget.h"
#include "persistentmodel/archive.h"

#include <ConsoleLog.h>
#include <TSettings.h>

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void about_window_menubar();
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
    qRegisterMetaType<QVector<File>>("QVector<File>");

    // Deal with PersistentStore
    PersistentStore::initializePersistentStore();
    PersistentStore &store = PersistentStore::instance();
    int              ok    = store.init();
    QVERIFY(ok);
}

void TestMainWindow::cleanupTestCase()
{
    PersistentStore::destroy();
    Translator::destroy();
    TSettings::destroy();
    ConsoleLog::destroy();

    // Wait for everything to finish
    QCoreApplication::processEvents(QEventLoop::AllEvents, 5000);
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
    MainWindow *    mainwindow = new MainWindow();
    HelpWidget *    help       = &mainwindow->_helpWidget;
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
        QVERIFY(help->_aboutWindow.isVisible() == true);
        QVERIFY(ui->aboutButton->isChecked() == true);
        VISUAL_WAIT;

        // Stay visible even when clicking the menu bar action again
        menuAction->trigger();
        QVERIFY(help->_aboutWindow.isVisible() == true);
        QVERIFY(ui->aboutButton->isChecked() == true);
        VISUAL_WAIT;

        // Becomes invisible by clicking the Help->About button
        ui->aboutButton->click();
        QVERIFY(help->_aboutWindow.isVisible() == false);
        QVERIFY(ui->aboutButton->isChecked() == false);
        VISUAL_WAIT;
    }

    delete mainwindow;
}

void TestMainWindow::quit_simple()
{
    MainWindow *mainwindow = new MainWindow();
    QSignalSpy  sig_getTaskInfo(mainwindow, SIGNAL(getTaskInfo()));

    VISUAL_INIT(mainwindow);

    // If we try to close the window, we emit a getTaskInfo instead
    QCloseEvent *close_event = new QCloseEvent();
    mainwindow->closeEvent(close_event);
    QVERIFY(sig_getTaskInfo.count() == 1);
    sig_getTaskInfo.clear();

    // Fake getting a reply which says there's no tasks.
    mainwindow->closeWithTaskInfo(false, 0, 0);

    // After quitting, we don't respond to more events.
    QCloseEvent *close_event_another = new QCloseEvent();
    mainwindow->closeEvent(close_event_another);
    QVERIFY(sig_getTaskInfo.count() == 0);

    delete mainwindow;
    delete close_event;
    delete close_event_another;
}

void TestMainWindow::quit_tasks()
{
    MainWindow *mainwindow = new MainWindow();
    QSignalSpy  sig_getTaskInfo(mainwindow, SIGNAL(getTaskInfo()));

    VISUAL_INIT(mainwindow);

    // Fake getting a response to a closeEvent (not sent in this test) which
    // says that there's running tasks, but cancel the quitting.
    QMetaObject::invokeMethod(mainwindow, "closeWithTaskInfo",
                              Qt::QueuedConnection, Q_ARG(bool, true),
                              Q_ARG(int, 1), Q_ARG(int, 1));
    QMetaObject::invokeMethod(&mainwindow->_stopTasksDialog, "close",
                              Qt::QueuedConnection);
    VISUAL_WAIT;

    // After cancelling the quit, we still respond to events
    QCloseEvent *close_event = new QCloseEvent();
    mainwindow->closeEvent(close_event);
    QVERIFY(sig_getTaskInfo.count() == 1);
    sig_getTaskInfo.clear();
    VISUAL_WAIT;

    // Quit the app
    // FIXME: sending an "accept" is a hack because the task-specific choices
    // (e.g., stop tasks, run in background) are only added to the dialog box
    // after MainWindow receives the closeWithTaskInfo, so we can't
    // "queue up" sending a message to one of those objects.
    QMetaObject::invokeMethod(mainwindow, "closeWithTaskInfo",
                              Qt::QueuedConnection, Q_ARG(bool, true),
                              Q_ARG(int, 1), Q_ARG(int, 1));
    QMetaObject::invokeMethod(&mainwindow->_stopTasksDialog, "accept",
                              Qt::QueuedConnection);
    VISUAL_WAIT;

    // After quitting, we don't respond to more events
    QCloseEvent *close_event_another = new QCloseEvent();
    mainwindow->closeEvent(close_event_another);
    QVERIFY(sig_getTaskInfo.count() == 0);
    VISUAL_WAIT;

    delete mainwindow;
    delete close_event;
    delete close_event_another;
}

void TestMainWindow::tab_navigation()
{
    MainWindow *    mainwindow = new MainWindow();
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
    MainWindow *       mainwindow    = new MainWindow();
    Ui::MainWindow *   ui            = mainwindow->_ui;
    JobsTabWidget *    jobsTabWidget = &mainwindow->_jobsTabWidget;
    Ui::JobsTabWidget *jui           = jobsTabWidget->_ui;

    BackupTabWidget *backupTabWidget = &mainwindow->_backupTabWidget;

    ArchiveWidget *archiveDetailsWidget =
        mainwindow->_archivesTabWidget._ui->archiveDetailsWidget;

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
    QMetaObject::invokeMethod(mainwindow, "browseForBackupItems",
                              Qt::QueuedConnection);
    QMetaObject::invokeMethod(&backupTabWidget->_filePickerDialog, "close",
                              Qt::QueuedConnection);
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
    Archive *  actual_archive = new Archive();
    ArchivePtr archive(actual_archive);
    archive->setName("Job_test-job_archive1");
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
    QSignalSpy sig_fileList(actual_archive, SIGNAL(fileList(QVector<File>)));
    mainwindow->displayInspectArchive(archive);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->archivesTab);
    VISUAL_WAIT;

    // Wait for archive parsing to finish
    while(sig_fileList.count() == 0)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    mainwindow->displayJobDetails(job);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->jobsTab);
    VISUAL_WAIT;

    // Click on the Job name in the archive tab.
    mainwindow->displayInspectArchive(archive);
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->archivesTab);
    VISUAL_WAIT;

    archiveDetailsWidget->jobClicked("test-job");
    QVERIFY(ui->mainTabWidget->currentWidget() == ui->jobsTab);
    VISUAL_WAIT;

    delete mainwindow;
}

QTEST_MAIN(TestMainWindow)
#include "test-mainwindow.moc"

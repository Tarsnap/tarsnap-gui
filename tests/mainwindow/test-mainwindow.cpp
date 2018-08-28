#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "mainwindow.h"

#define VISUAL_INIT                                                            \
    IF_VISUAL                                                                  \
    {                                                                          \
        mainwindow->show();                                                    \
        VISUAL_WAIT;                                                           \
    }

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void about_window();
    void console_window();
    void quit_simple();
    void quit_tasks();
};

void TestMainWindow::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }
}

static QAction *get_menubar_about(QMenuBar *menubar)
{
    foreach(QAction *action, menubar->actions())
    {
        if(action->menu())
        {
            foreach(QAction *subaction, action->menu()->actions())
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

void TestMainWindow::about_window()
{
    MainWindow *   mainwindow = new MainWindow();
    Ui::MainWindow ui         = mainwindow->_ui;

    VISUAL_INIT;

    // Starts off not visible and the button is not pushed down
    ui.actionGoHelp->trigger();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
    QVERIFY(ui.aboutButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui.aboutButton->click();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
    QVERIFY(ui.aboutButton->isChecked() == true);
    VISUAL_WAIT;

    // Becomes invisible by clicking the button again
    ui.aboutButton->click();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
    QVERIFY(ui.aboutButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui.aboutButton->click();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
    QVERIFY(ui.aboutButton->isChecked() == true);
    VISUAL_WAIT;

    // Becomes invisible by closing the About window
    mainwindow->_aboutWindow.close();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
    QVERIFY(ui.aboutButton->isChecked() == false);
    VISUAL_WAIT;

    // Test the "About Tarsnap" menubar item, if applicable.
    if(mainwindow->_menuBar != nullptr)
    {
        // find "About Tarsnap" menu item
        QAction *menuAction = get_menubar_about(mainwindow->_menuBar);
        QVERIFY(menuAction != nullptr);
        VISUAL_WAIT;

        // Becomes visible using the menu bar action
        menuAction->trigger();
        QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
        QVERIFY(ui.aboutButton->isChecked() == true);
        VISUAL_WAIT;

        // Stay visible even when clicking the menu bar action again
        menuAction->trigger();
        QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
        QVERIFY(ui.aboutButton->isChecked() == true);
        VISUAL_WAIT;

        // Becomes invisible by clicking the Help->About button
        ui.aboutButton->click();
        QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
        QVERIFY(ui.aboutButton->isChecked() == false);
        VISUAL_WAIT;
    }

    delete mainwindow;
}

void TestMainWindow::quit_simple()
{
    MainWindow *mainwindow = new MainWindow();
    QSignalSpy  sig_getTaskInfo(mainwindow, SIGNAL(getTaskInfo()));

    VISUAL_INIT;

    // If we try to close the window, we emit a getTaskInfo instead
    mainwindow->closeEvent(new QCloseEvent());
    QVERIFY(sig_getTaskInfo.count() == 1);
    sig_getTaskInfo.clear();

    // Fake getting a reply which says there's no tasks.
    mainwindow->closeWithTaskInfo(false, 0, 0);

    // After quitting, we don't respond to more events.
    mainwindow->closeEvent(new QCloseEvent());
    QVERIFY(sig_getTaskInfo.count() == 0);

    delete mainwindow;
}

void TestMainWindow::quit_tasks()
{
    MainWindow *mainwindow = new MainWindow();
    QSignalSpy  sig_getTaskInfo(mainwindow, SIGNAL(getTaskInfo()));

    VISUAL_INIT;

    // Fake getting a response to a closeEvent (not sent in this test) which
    // says that there's running tasks, but cancel the quitting.
    QMetaObject::invokeMethod(mainwindow, "closeWithTaskInfo",
                              Qt::QueuedConnection, Q_ARG(bool, true),
                              Q_ARG(int, 1), Q_ARG(int, 1));
    QMetaObject::invokeMethod(&mainwindow->_stopTasksDialog, "close",
                              Qt::QueuedConnection);
    VISUAL_WAIT;

    // After cancelling the quit, we still respond to events
    mainwindow->closeEvent(new QCloseEvent());
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
    mainwindow->closeEvent(new QCloseEvent());
    QVERIFY(sig_getTaskInfo.count() == 0);
    VISUAL_WAIT;

    delete mainwindow;
}

void TestMainWindow::console_window()
{
    MainWindow *   mainwindow = new MainWindow();
    Ui::MainWindow ui         = mainwindow->_ui;

    VISUAL_INIT;

    // Starts off not visible and the button is not pushed down
    ui.actionGoHelp->trigger();
    QVERIFY(mainwindow->_consoleWindow.isVisible() == false);
    QVERIFY(ui.consoleButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui.consoleButton->click();
    QVERIFY(mainwindow->_consoleWindow.isVisible() == true);
    QVERIFY(ui.consoleButton->isChecked() == true);
    VISUAL_WAIT;

    // Becomes invisible by clicking the button again
    ui.consoleButton->click();
    QVERIFY(mainwindow->_consoleWindow.isVisible() == false);
    QVERIFY(ui.consoleButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui.consoleButton->click();
    QVERIFY(mainwindow->_consoleWindow.isVisible() == true);
    QVERIFY(ui.consoleButton->isChecked() == true);
    VISUAL_WAIT;

    // Becomes invisible by closing the Console window
    mainwindow->_consoleWindow.close();
    QVERIFY(mainwindow->_consoleWindow.isVisible() == false);
    QVERIFY(ui.consoleButton->isChecked() == false);
    VISUAL_WAIT;

    delete mainwindow;
}

QTEST_MAIN(TestMainWindow)
#include "test-mainwindow.moc"

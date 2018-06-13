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

public:
    TestMainWindow();
    ~TestMainWindow();

private slots:
    void initTestCase();
    void about_window();

private:
};

TestMainWindow::TestMainWindow()
{
    QCoreApplication::setOrganizationName(TEST_NAME);
}

TestMainWindow::~TestMainWindow()
{
}

void TestMainWindow::initTestCase()
{
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

QTEST_MAIN(TestMainWindow)
#include "main.moc"

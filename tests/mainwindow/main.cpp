#include <QtTest/QtTest>

#include "mainwindow.h"

class TestMainWindow : public QObject
{
    Q_OBJECT

public:
    TestMainWindow();
    ~TestMainWindow();

private slots:
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

    // Starts off not visible and the button is not pushed down
    QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
    QVERIFY(ui.aboutButton->isChecked() == false);

    // Becomes visible
    ui.aboutButton->click();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
    QVERIFY(ui.aboutButton->isChecked() == true);

    // Becomes invisible by clicking the button again
    ui.aboutButton->click();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
    QVERIFY(ui.aboutButton->isChecked() == false);

    // Becomes visible
    ui.aboutButton->click();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
    QVERIFY(ui.aboutButton->isChecked() == true);

    // Becomes invisible by closing the About window
    mainwindow->_aboutWindow.close();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
    QVERIFY(ui.aboutButton->isChecked() == false);

    // Test the "About Tarsnap" menubar item, if applicable.
    if(mainwindow->_menuBar != nullptr)
    {
        // find "About Tarsnap" menu item
        QAction *menuAction = get_menubar_about(mainwindow->_menuBar);
        QVERIFY(menuAction != nullptr);

        // Becomes visible using the menu bar action
        menuAction->trigger();
        QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
        QVERIFY(ui.aboutButton->isChecked() == true);

        // Stay visible even when clicking the menu bar action again
        menuAction->trigger();
        QVERIFY(mainwindow->_aboutWindow.isVisible() == true);
        QVERIFY(ui.aboutButton->isChecked() == true);

        // Becomes invisible by clicking the Help->About button
        ui.aboutButton->click();
        QVERIFY(mainwindow->_aboutWindow.isVisible() == false);
        QVERIFY(ui.aboutButton->isChecked() == false);
    }

    delete mainwindow;
}

QTEST_MAIN(TestMainWindow)
#include "main.moc"

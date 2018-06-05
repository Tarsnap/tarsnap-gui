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

#ifdef Q_OS_OSX
    // find "About Tarsnap" menu item
    QAction *menuAction = nullptr;
    foreach(QAction *action, mainwindow->_menuBar->actions())
    {
        if(action->menu())
        {
            foreach(QAction *subaction, action->menu()->actions())
            {
                if(subaction->menuRole() == QAction::AboutRole)
                {
                    menuAction = subaction;
                    break;
                }
            }
        }
    }

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
#endif

    delete mainwindow;
}

QTEST_MAIN(TestMainWindow)
#include "main.moc"

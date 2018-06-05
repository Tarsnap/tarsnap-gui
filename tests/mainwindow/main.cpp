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

    // Starts off not visible
    QVERIFY(mainwindow->_aboutWindow.isVisible() == false);

    // Becomes visible
    ui.aboutButton->click();
    QVERIFY(mainwindow->_aboutWindow.isVisible() == true);

    delete mainwindow;
}

QTEST_MAIN(TestMainWindow)
#include "main.moc"

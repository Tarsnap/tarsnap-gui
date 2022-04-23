#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QObject>
#include <QTest>

#include "ui_helpwidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "TPopupPushButton.h"

#include "widgets/aboutdialog.h"
#include "widgets/consolelogdialog.h"
#include "widgets/helpwidget.h"

#define LONG_TEXT                                                              \
    "This is a long piece of text which should prompt line wrapping when it"   \
    " it placed inside a QPlainTextEdit."

class TestHelpWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void helpwidget();
    void about_window();
    void console_window();
};

void TestHelpWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }
}

void TestHelpWidget::helpwidget()
{
    HelpWidget *helpWidget = new HelpWidget();

    VISUAL_INIT(helpWidget);

    delete helpWidget;
}

void TestHelpWidget::about_window()
{
    HelpWidget     *helpWidget = new HelpWidget();
    Ui::HelpWidget *ui         = helpWidget->_ui;

    VISUAL_INIT(helpWidget);

    // Starts off not visible and the button is not pushed down
    QVERIFY(helpWidget->_aboutWindow->isVisible() == false);
    QVERIFY(ui->aboutButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui->aboutButton->click();
    QVERIFY(helpWidget->_aboutWindow->isVisible() == true);
    QVERIFY(ui->aboutButton->isChecked() == true);
    VISUAL_WAIT;

    // Becomes invisible by clicking the button again
    ui->aboutButton->click();
    QVERIFY(helpWidget->_aboutWindow->isVisible() == false);
    QVERIFY(ui->aboutButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui->aboutButton->click();
    QVERIFY(helpWidget->_aboutWindow->isVisible() == true);
    QVERIFY(ui->aboutButton->isChecked() == true);
    VISUAL_WAIT;

    // Becomes invisible by closing the About window
    helpWidget->_aboutWindow->close();
    QVERIFY(helpWidget->_aboutWindow->isVisible() == false);
    QVERIFY(ui->aboutButton->isChecked() == false);
    VISUAL_WAIT;

    delete helpWidget;
}

void TestHelpWidget::console_window()
{
    ConsoleLogDialog *consoleWindow = new ConsoleLogDialog();

    VISUAL_INIT(consoleWindow);

    // Starts off visible
    QVERIFY(consoleWindow->isVisible() == true);
    VISUAL_WAIT;

    // Becomes invisible
    consoleWindow->hide();
    QVERIFY(consoleWindow->isVisible() == false);
    VISUAL_WAIT;

    // Becomes visible
    consoleWindow->show();
    QVERIFY(consoleWindow->isVisible() == true);
    VISUAL_WAIT;

    // Add some text to the consoleLog
    consoleWindow->appendLogString(LONG_TEXT);
    VISUAL_WAIT;

    // Becomes invisible by closing the Console window
    consoleWindow->close();
    QVERIFY(consoleWindow->isVisible() == false);
    VISUAL_WAIT;

    // Add more text while invisible
    consoleWindow->appendLogString(LONG_TEXT);
    VISUAL_WAIT;

    // Become visible again
    consoleWindow->show();
    QVERIFY(consoleWindow->isVisible() == true);
    VISUAL_WAIT;

    delete consoleWindow;
}

QTEST_MAIN(TestHelpWidget)
WARNINGS_DISABLE
#include "test-helpwidget.moc"
WARNINGS_ENABLE

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDialog>
#include <QObject>
#include <QTest>

#include "ui_helpwidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "TPopupPushButton.h"

#include "widgets/helpwidget.h"

#define LONG_TEXT                                                              \
    "This is a long piece of text which should prompt line wrapping when it"   \
    " it placed inside a QPlainTextEdit."

class TestHelpWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void about_window();
    void console_window();
};

void TestHelpWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }
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
    HelpWidget     *helpWidget = new HelpWidget();
    Ui::HelpWidget *ui         = helpWidget->_ui;

    VISUAL_INIT(helpWidget);

    // Starts off not visible and the button is not pushed down
    QVERIFY(helpWidget->_consoleWindow->isVisible() == false);
    QVERIFY(ui->consoleButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui->consoleButton->click();
    QVERIFY(helpWidget->_consoleWindow->isVisible() == true);
    QVERIFY(ui->consoleButton->isChecked() == true);
    VISUAL_WAIT;

    // Becomes invisible by clicking the button again
    ui->consoleButton->click();
    QVERIFY(helpWidget->_consoleWindow->isVisible() == false);
    QVERIFY(ui->consoleButton->isChecked() == false);
    VISUAL_WAIT;

    // Becomes visible
    ui->consoleButton->click();
    QVERIFY(helpWidget->_consoleWindow->isVisible() == true);
    QVERIFY(ui->consoleButton->isChecked() == true);
    VISUAL_WAIT;

    // Add some text to the consoleLog
    helpWidget->appendLogString(LONG_TEXT);
    VISUAL_WAIT;

    // Becomes invisible by closing the Console window
    helpWidget->_consoleWindow->close();
    QVERIFY(helpWidget->_consoleWindow->isVisible() == false);
    QVERIFY(ui->consoleButton->isChecked() == false);
    VISUAL_WAIT;

    // Add more text while invisible
    helpWidget->appendLogString(LONG_TEXT);
    VISUAL_WAIT;

    // Become visible again
    ui->consoleButton->click();
    QVERIFY(helpWidget->_consoleWindow->isVisible() == true);
    QVERIFY(ui->consoleButton->isChecked() == true);
    VISUAL_WAIT;

    delete helpWidget;
}

QTEST_MAIN(TestHelpWidget)
WARNINGS_DISABLE
#include "test-helpwidget.moc"
WARNINGS_ENABLE

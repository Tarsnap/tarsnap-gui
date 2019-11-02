#include "helpwidget.h"

WARNINGS_DISABLE
#include <QDesktopServices>
#include <QWidget>

#include "ui_aboutwidget.h"
#include "ui_consolewidget.h"
#include "ui_helpwidget.h"
WARNINGS_ENABLE

#include "debug.h"

HelpWidget::HelpWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::HelpWidget)
{
    // Ui initialization
    _ui->setupUi(this);

    // Initialize the Help tab text
    QFile helpTabFile(":/text/help-tab.xml");
    if(!helpTabFile.open(QFile::ReadOnly | QIODevice::Text))
    {
        DEBUG << "Failed to load a resource file.";
    }
    _helpTabHTML = QTextStream(&helpTabFile).readAll();
    helpTabFile.close();

    // After reading the Help tab text
    updateUi();

    // Initialize About window
    Ui::aboutWidget aboutUi;
    aboutUi.setupUi(&_aboutWindow);
    aboutUi.versionLabel->setText(tr("GUI version ")
                                  + QCoreApplication::applicationVersion());
    _aboutWindow.setWindowFlags(
        (_aboutWindow.windowFlags() | Qt::CustomizeWindowHint)
        & ~Qt::WindowMaximizeButtonHint);
    connect(aboutUi.checkUpdateButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(
            QUrl("https://github.com/Tarsnap/tarsnap-gui/releases"));
    });
    _ui->aboutButton->setPopup(&_aboutWindow);

    // Initialize console log
    Ui::consoleWidget consoleUI;
    consoleUI.setupUi(&_consoleWindow);
    _consoleLog = consoleUI.log;
    _ui->consoleButton->setPopup(&_consoleWindow);
}

HelpWidget::~HelpWidget()
{
    delete _ui;
}

QPlainTextEdit *HelpWidget::getConsoleLog()
{
    return _consoleLog;
}

void HelpWidget::aboutMenuClicked()
{
    // This always displays the About window
    _ui->aboutButton->setChecked(true);
}

void HelpWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void HelpWidget::updateUi()
{
    // Keyboard shortcuts
    _ui->helpTabText->setHtml(
        _helpTabHTML
            .arg(QKeySequence(Qt::ControlModifier)
                     .toString(QKeySequence::NativeText))
            .arg(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier)
                     .toString(QKeySequence::NativeText))
            .arg(QKeySequence(Qt::Key_Backspace)
                     .toString(QKeySequence::NativeText))
            .arg(QKeySequence(Qt::Key_Delete)
                     .toString(QKeySequence::NativeText)));
}

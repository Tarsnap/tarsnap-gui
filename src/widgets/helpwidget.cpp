#include "helpwidget.h"

WARNINGS_DISABLE
#include <QEvent>
#include <QFile>
#include <QIODevice>
#include <QKeySequence>
#include <QTextBrowser>
#include <QTextStream>
#include <QWidget>
#include <Qt>

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
    updateKeyboardShortcutInfo();
}

HelpWidget::~HelpWidget()
{
    delete _ui;
}

void HelpWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateKeyboardShortcutInfo();
    }
    QWidget::changeEvent(event);
}

void HelpWidget::updateKeyboardShortcutInfo()
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

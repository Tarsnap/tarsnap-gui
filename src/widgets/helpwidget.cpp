#include "helpwidget.h"

#include <QWidget>

HelpWidget::HelpWidget(QWidget *parent) : QWidget(parent)
{
    // Ui initialization
    _ui.setupUi(this);
}

void HelpWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui.retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void HelpWidget::updateUi()
{
}

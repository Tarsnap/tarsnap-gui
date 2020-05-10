#include "statusbarwidget.h"

WARNINGS_DISABLE
#include <QEvent>
#include <QWidget>

#include "ui_statusbarwidget.h"
WARNINGS_ENABLE

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::StatusBarWidget)
{
    // Ui initialization
    _ui->setupUi(this);
    updateUi();
}

StatusBarWidget::~StatusBarWidget()
{
    delete _ui;
}

void StatusBarWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void StatusBarWidget::updateUi()
{
}

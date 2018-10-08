#include "jobstabwidget.h"

#include <QWidget>

#include <TSettings.h>

JobsTabWidget::JobsTabWidget(QWidget *parent) : QWidget(parent)
{
    // Ui initialization
    _ui.setupUi(this);

    loadSettings();
}

void JobsTabWidget::loadSettings()
{
    TSettings settings;
}

void JobsTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui.retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void JobsTabWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    default:
        QWidget::keyPressEvent(event);
    }
}

void JobsTabWidget::updateUi()
{
}

#include "archivestabwidget.h"

#include <QSettings>
#include <QWidget>

ArchivesTabWidget::ArchivesTabWidget(QWidget *parent) : QWidget(parent)
{
    // Ui initialization
    _ui.setupUi(this);
}

void ArchivesTabWidget::loadSettings()
{
    QSettings settings;
}

void ArchivesTabWidget::commitSettings()
{
    QSettings settings;

    settings.sync();
}

void ArchivesTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui.retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void ArchivesTabWidget::updateUi()
{
}

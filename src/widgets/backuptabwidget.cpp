#include "backuptabwidget.h"

#include <QSettings>
#include <QWidget>

BackupTabWidget::BackupTabWidget(QWidget *parent) : QWidget(parent)
{
    // Ui initialization
    _ui.setupUi(this);
}

void BackupTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui.retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void BackupTabWidget::updateUi()
{
}

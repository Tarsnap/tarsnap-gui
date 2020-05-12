#include "statisticsdialog.h"

WARNINGS_DISABLE
#include <QWidget>

#include "ui_statisticsdialog.h"
WARNINGS_ENABLE

#include "TSettings.h"

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::StatisticsDialog)
{
    // Ui initialization
    _ui->setupUi(this);
    updateUi();
}

StatisticsDialog::~StatisticsDialog()
{
    delete _ui;
}

void StatisticsDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void StatisticsDialog::updateUi()
{
}

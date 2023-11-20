#include "TProgressWidget.h"

WARNINGS_DISABLE
#include <QProgressBar>

#include "ui_TProgressWidget.h"
WARNINGS_ENABLE

#include "TBusyLabel.h"
#include "TElidedLabel.h"

TProgressWidget::TProgressWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::TProgressWidget)
{
    _ui->setupUi(this);
}

TProgressWidget::~TProgressWidget()
{
}

void TProgressWidget::messageNormal(const QString &text)
{
    _ui->statusLabel->messageNormal(text);
}

void TProgressWidget::messageError(const QString &text)
{
    _ui->statusLabel->messageError(text);
}

void TProgressWidget::setBusy(bool active)
{
    _ui->busyLabel->animate(active);
}

void TProgressWidget::setValue(int value)
{
    _ui->progressBar->setValue(value);
}

void TProgressWidget::clear()
{
    _ui->statusLabel->clear();
    _ui->busyLabel->animate(false);
    _ui->progressBar->setValue(0);
}

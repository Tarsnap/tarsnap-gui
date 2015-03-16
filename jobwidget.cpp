#include "jobwidget.h"

JobWidget::JobWidget(QWidget *parent) : QWidget(parent), _ui(new Ui::JobWidget)
{
    _ui->setupUi(this);
    _ui->stackedWidget->hide();
}

JobWidget::~JobWidget()
{
    delete _ui;
}

void JobWidget::focusInEvent(QFocusEvent *event)
{
    _ui->stackedWidget->show();
    adjustSize();
    QWidget::focusInEvent(event);
    _ui->nameLabel->clear();
}

void JobWidget::​focusOutEvent(QFocusEvent *event)
{
    _ui->stackedWidget->hide();
    adjustSize();
    QWidget::focusOutEvent(event);
}

void JobWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    _ui->stackedWidget->show();
    adjustSize();
}


void JobWidget::on_browseButton_clicked()
{
    _ui->stackedWidget->setCurrentWidget(_ui->restoreView);
}

void JobWidget::on_backButton_clicked()
{
    _ui->stackedWidget->setCurrentWidget(_ui->treeView);
}

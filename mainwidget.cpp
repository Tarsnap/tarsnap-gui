#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    QLabel *tarsnapLogo = new QLabel(this);
    tarsnapLogo->setText("Tarsnap");
    tarsnapLogo->setStyleSheet("font: italic \"Lucida\"; font-size: 22pt ;");
    tarsnapLogo->move(this->width()-tarsnapLogo->width()-10,0);
    tarsnapLogo->show();
}

MainWidget::~MainWidget()
{
    delete ui;
}

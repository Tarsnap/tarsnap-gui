#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QPainter>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    QLabel *tarsnapLogo = new QLabel(this);
    tarsnapLogo->setText("Tarsnappy");
    tarsnapLogo->resize(150, 30);
    tarsnapLogo->setStyleSheet("font: italic \"Lucida\"; font-size: 22pt ;");
    tarsnapLogo->move(this->width()-tarsnapLogo->width(),0);
    tarsnapLogo->show();
}

MainWidget::~MainWidget()
{
    delete ui;
}


void MainWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QPainter>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    m_tarsnapLogo = new QLabel(this);
    m_tarsnapLogo->setText("Tarsnappy");
    m_tarsnapLogo->resize(150, 30);
    m_tarsnapLogo->setStyleSheet("font: italic \"Lucida\"; font-size: 22pt ;");
    m_tarsnapLogo->show();
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

    m_tarsnapLogo->move(this->width()-m_tarsnapLogo->width(),3);
}

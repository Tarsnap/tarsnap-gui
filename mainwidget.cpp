#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "ui_restoreitemwidget.h"
#include "ui_backupitemwidget.h"

#include <QPainter>
#include <QDebug>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    _tarsnapLogo = new QLabel(this);
    QPixmap logo(":/resources/tarsnap.png");
    _tarsnapLogo->setPixmap(logo.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _tarsnapLogo->adjustSize();
//    m_tarsnapLogo->setText("Tarsnappy");
//    m_tarsnapLogo->setStyleSheet("font: italic \"Lucida Grande\"; font-size: 22pt ;");
    _tarsnapLogo->show();

    Ui::RestoreItemWidget restoreItemUi;
    for(int i = 0; i < 10; i++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        QWidget *widget = new QWidget;
        restoreItemUi.setupUi(widget);
        ui->backupRestoreListWidget->insertItem(i, item);
        ui->backupRestoreListWidget->setItemWidget(item, widget);
    }

    Ui::BackupItemWidget backupItemUi;
    QStringList paths;
    paths << "/home/shinnok/" << "/srv/backups/backup_27.12.2015.tgz" << "/home/bill/.vimrc"
          << "/home/bob/student_grades.xslt";
    foreach(QString path, paths)
    {
        QListWidgetItem *item = new QListWidgetItem;
        QWidget *widget = new QWidget;
        backupItemUi.setupUi(widget);
        backupItemUi.pathLabel->setText(path);
        ui->backupListWidget->insertItem(0, item);
        ui->backupListWidget->setItemWidget(item, widget);
    }
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

    _tarsnapLogo->move(this->width()-_tarsnapLogo->width()-10,3);
}

void MainWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _windowDragPos = event->pos();
    }
}

void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - _windowDragPos;
        QPoint newpos = this->pos() + diff;
        this->move(newpos);
    }
}

void MainWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(isMaximized())
        this->showNormal();
    else
        this->showMaximized();
}

#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "ui_restoreitemwidget.h"
#include "ui_backupitemwidget.h"

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
        backupItemUi.path->setText(path);
        ui->onDemandBackupListWidget->insertItem(0, item);
        ui->onDemandBackupListWidget->setItemWidget(item, widget);
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

    m_tarsnapLogo->move(this->width()-m_tarsnapLogo->width(),3);
}

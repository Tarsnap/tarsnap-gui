#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_restoreitemwidget.h"
#include "ui_backupitemwidget.h"

#include <QPainter>
#include <QDebug>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _tarsnapLogo = new QLabel(this);
    QPixmap logo(":/resources/tarsnap.png");
    _tarsnapLogo->setPixmap(logo.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    _tarsnapLogo->adjustSize();
    _tarsnapLogo->show();

    readSettings();

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
    paths << "/home/shinnok/" << "/srv/backups/backup_27.12.2015.tgz"
          << "/home/bill/.vimrc" << "/home/bob/student_grades.xslt";
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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readSettings()
{
    QSettings settings;
    ui->tarsnapUserLineEdit->setText(settings.value("tarsnap/user", "").toString());
    ui->tarsnapKeyLineEdit->setText(settings.value("tarsnap/key", "").toString());
    ui->tarsnapMachineLineEdit->setText(settings.value("tarsnap/machine", "").toString());
    ui->tarsnapPathLineEdit->setText(settings.value("tarsnap/path", "").toString());
    ui->tarsnapCacheLineEdit->setText(settings.value("tarsnap/cache", "").toString());
}


void MainWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    _tarsnapLogo->move(this->width()-_tarsnapLogo->width()-10,3);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _windowDragPos = event->pos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - _windowDragPos;
        QPoint newpos = this->pos() + diff;
        this->move(newpos);
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(isMaximized())
        this->showNormal();
    else
        this->showMaximized();
}

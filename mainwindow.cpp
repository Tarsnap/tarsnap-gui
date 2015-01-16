#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_restoreitemwidget.h"
#include "ui_backupitemwidget.h"

#include <QPainter>
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    _ui->backupListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

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
        _ui->backupRestoreListWidget->insertItem(i, item);
        _ui->backupRestoreListWidget->setItemWidget(item, widget);
    }

//    Ui::BackupItemWidget backupItemUi;
//    QStringList paths;
//    paths << "/home/shinnok/" << "/srv/backups/backup_27.12.2015.tgz"
//          << "/home/bill/.vimrc" << "/home/bob/student_grades.xslt";
//    foreach(QString path, paths)
//    {
//        QListWidgetItem *item = new QListWidgetItem;
//        QWidget *widget = new QWidget;
//        backupItemUi.setupUi(widget);
//        backupItemUi.pathLabel->setText(path);
//        ui->backupListWidget->insertItem(0, item);
//        ui->backupListWidget->setItemWidget(item, widget);
//    }
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::readSettings()
{
    QSettings settings;
    _ui->tarsnapUserLineEdit->setText(settings.value("tarsnap/user", "").toString());
    _ui->tarsnapKeyLineEdit->setText(settings.value("tarsnap/key", "").toString());
    _ui->tarsnapMachineLineEdit->setText(settings.value("tarsnap/machine", "").toString());
    _ui->tarsnapPathLineEdit->setText(settings.value("tarsnap/path", "").toString());
    _ui->tarsnapCacheLineEdit->setText(settings.value("tarsnap/cache", "").toString());
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

void MainWindow::on_appendTimestampCheckBox_toggled(bool checked)
{
    if(checked)
    {
        QString text = _ui->backupNameLineEdit->text();
        text.append("_");
        text.append(QDateTime::currentDateTime().toString("dd.MM.yyyy_HH:mm:ss"));
        _ui->backupNameLineEdit->setText(text);
        _ui->backupNameLineEdit->setCursorPosition(0);
    }
}

void MainWindow::on_backupListInfoLabel_linkActivated(const QString &link)
{
    // Can't select multiple directories and files at the same time using the Native dialog
    // Thus instead of being able to select only dirs or files, we'll be using a custom
    // Qt dialog for now
    /*
    QStringList paths = QFileDialog::getOpenFileNames(this,
                                                      tr("Select files and directories")
                                                      , QDir::homePath());
                                                      */
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::DontUseNativeDialog,true);
    QListView *l = dialog.findChild<QListView*>("listView");
    if(l)
        l->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView *t = dialog.findChild<QTreeView*>();
    if(t)
        t->setSelectionMode(QAbstractItemView::MultiSelection);
    if(dialog.exec())
        QMetaObject::invokeMethod(_ui->backupListWidget, "addItemsWithUrls", Qt::QueuedConnection, Q_ARG(QList<QUrl>, dialog.selectedUrls()));
//    qDebug() << dialog.selectedUrls();
}

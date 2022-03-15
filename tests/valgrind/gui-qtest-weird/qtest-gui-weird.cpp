#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFileSystemModel>
#include <QFontMetrics>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QSequentialAnimationGroup>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>
WARNINGS_ENABLE

class TestQTestWeird : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
    void pl_setvisible();
    void pl_stackedwidget1();
    void pl_stackedwidget2();
    void pl_messagebox_icon();
    void pl_messagebox_buttons();
    void pl_messagebox_exec();
    void pl_saveGeometry();
};

void TestQTestWeird::pl_nothing()
{
}

void TestQTestWeird::pl_setvisible()
{
    QWidget *wid = new QWidget();
    wid->setLayout(new QVBoxLayout());

    QWidget *p1 = new QWidget();
    QWidget *p2 = new QWidget();
    wid->layout()->addWidget(p1);
    wid->layout()->addWidget(p2);

    // We only need to set this on one of them.
    p1->setLayout(new QVBoxLayout());

    wid->show();
    p1->hide();
    p1->show();

    delete wid;
}

void TestQTestWeird::pl_stackedwidget1()
{
    QWidget *wid = new QWidget();
    wid->setLayout(new QVBoxLayout());
    QStackedWidget *p1 = new QStackedWidget();
    QStackedWidget *p2 = new QStackedWidget();
    wid->layout()->addWidget(p1);
    wid->layout()->addWidget(p2);

    QWidget *hideme = new QWidget();
    hideme->setLayout(new QVBoxLayout());
    QWidget *inner = new QWidget();
    inner->setLayout(new QVBoxLayout());
    inner->layout()->addWidget(new QLabel());
    inner->layout()->addWidget(new QLineEdit());
    hideme->layout()->addWidget(inner);
    hideme->hide();

    wid->layout()->addWidget(hideme);

    wid->show();

    hideme->show();
    hideme->hide();
    p1->hide();
    p1->show();
    p2->hide();
    p2->show();

    delete wid;
}

void TestQTestWeird::pl_stackedwidget2()
{
    QStackedWidget *wid = new QStackedWidget();

    QStackedWidget *p1 = new QStackedWidget();
    QStackedWidget *p2 = new QStackedWidget();
    wid->addWidget(p1);
    wid->addWidget(p2);

    QWidget *hideme = new QWidget();
    hideme->setLayout(new QVBoxLayout());
    QWidget *inner = new QWidget();
    inner->setLayout(new QVBoxLayout());
    inner->layout()->addWidget(new QLabel());
    inner->layout()->addWidget(new QLineEdit());
    hideme->layout()->addWidget(inner);
    hideme->hide();

    p1->addWidget(hideme);

    wid->show();
    wid->setCurrentIndex(1);
    wid->setCurrentIndex(0);

    delete wid;
}

void TestQTestWeird::pl_messagebox_icon()
{
    QMessageBox *box = new QMessageBox();
    box->setIcon(QMessageBox::Critical);
    delete box;
}

void TestQTestWeird::pl_messagebox_buttons()
{
    QMessageBox *box = new QMessageBox();
    box->setStandardButtons(QMessageBox::Cancel);
    delete box;
}

void TestQTestWeird::pl_messagebox_exec()
{
    QMessageBox *box = new QMessageBox();
    QMetaObject::invokeMethod(box, "close", Qt::QueuedConnection);
    box->exec();
    delete box;
}

void TestQTestWeird::pl_saveGeometry()
{
    QWidget *wid = new QWidget();
    wid->saveGeometry();
    delete wid;
}

QTEST_MAIN(TestQTestWeird)
#include "qtest-gui-weird.moc"

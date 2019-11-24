#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QCollator>
#include <QDialog>
#include <QFont>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QWidget>
WARNINGS_ENABLE

class TestQTestSimple : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
    void pl_widget();
    void pl_font();
    void pl_qapp_font();
    void pl_label();
    void pl_collator();
    void pl_collator_copy();
    void pl_listwidget();
    void pl_plaintextedit();
};

void TestQTestSimple::pl_nothing()
{
}

void TestQTestSimple::pl_widget()
{
    QWidget *widget = new QWidget();
    delete widget;
}

void TestQTestSimple::pl_font()
{
    QFont *font = new QFont();
    delete font;
}

void TestQTestSimple::pl_qapp_font()
{
    QFont font = qApp->font();
}

void TestQTestSimple::pl_label()
{
    QLabel *label = new QLabel();
    label->setText("Let's cause a memory leak!");
    delete label;
}

void TestQTestSimple::pl_collator()
{
    QCollator *coll = new QCollator();
    delete coll;
}

void TestQTestSimple::pl_collator_copy()
{
    QCollator *coll      = new QCollator();
    QCollator *coll_copy = new QCollator(*coll);
    delete coll_copy;
    delete coll;
}

void TestQTestSimple::pl_listwidget()
{
    QListWidget *lw = new QListWidget();
    delete lw;
}

void TestQTestSimple::pl_plaintextedit()
{
    QPlainTextEdit *te = new QPlainTextEdit();
    delete te;
}

QTEST_MAIN(TestQTestSimple)
#include "qtest-gui-simple.moc"

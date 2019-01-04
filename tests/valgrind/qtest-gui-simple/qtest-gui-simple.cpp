#include <QtTest/QtTest>

#include <QCollator>
#include <QDialog>
#include <QFont>
#include <QLabel>
#include <QWidget>

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

QTEST_MAIN(TestQTestSimple)
#include "qtest-gui-simple.moc"

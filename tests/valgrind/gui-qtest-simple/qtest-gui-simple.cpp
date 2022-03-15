#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QAccessible>
#include <QCollator>
#include <QDialog>
#include <QFont>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QImageReader>
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
    void pl_accessible_active();
    void pl_widget();
    void pl_accessible_setRootObject();
    void pl_font();
    void pl_qapp_font();
    void pl_fontmetrics_height();
    void pl_fontmetricsF_height();
    void pl_fontmetrics_lineSpacing();
    void pl_fontmetricsf_leading();
    void pl_label();
    void pl_collator();
    void pl_collator_copy();
    void pl_listwidget();
    void pl_plaintextedit();
    void pl_image_supported();
};

void TestQTestSimple::pl_nothing()
{
}

void TestQTestSimple::pl_accessible_active()
{
    volatile bool access = QAccessible::isActive();
    (void)access; /* UNUSED */
}

void TestQTestSimple::pl_widget()
{
    QWidget *widget = new QWidget();
    delete widget;
}

void TestQTestSimple::pl_accessible_setRootObject()
{
    QWidget *widget = new QWidget();
    QAccessible::setRootObject(widget);
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

void TestQTestSimple::pl_fontmetrics_height()
{
    QFontMetrics *fm = new QFontMetrics(qApp->font());
    fm->height();
    fm->lineSpacing();
    delete fm;
}

void TestQTestSimple::pl_fontmetricsF_height()
{
    QFontMetricsF *fm = new QFontMetricsF(qApp->font());
    fm->height();
    delete fm;
}

void TestQTestSimple::pl_fontmetrics_lineSpacing()
{
    QFontMetrics *fm = new QFontMetrics(qApp->font());
    fm->lineSpacing();
    delete fm;
}

void TestQTestSimple::pl_fontmetricsf_leading()
{
    // Yes, the previous one doesn't have the trailing F, and no, I don't know
    // precisely why both are necessary.
    QFontMetricsF *fm = new QFontMetricsF(qApp->font());
    fm->leading();
    delete fm;
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

void TestQTestSimple::pl_image_supported()
{
    QImageReader::supportedImageFormats();
}

QTEST_MAIN(TestQTestSimple)
#include "qtest-gui-simple.moc"

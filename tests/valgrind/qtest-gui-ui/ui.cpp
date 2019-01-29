#include <QtTest/QtTest>

#include "ui_filepickerwidget.h"

class TestUi : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();

    void pl_filepickerwidget();
};

void TestUi::pl_nothing()
{
}

void TestUi::pl_filepickerwidget()
{
    QWidget *             wid = new QWidget();
    Ui::FilePickerWidget *ui  = new Ui::FilePickerWidget;

    // main source of memleaks
    ui->setupUi(wid);
    // retranslateUi() is called from setupUi(), but calling it again produces
    // another memleak.  Our main code does call it as part of the translation
    // layer, so we need to do so here.
    ui->retranslateUi(wid);

    delete ui;
    delete wid;
}

QTEST_MAIN(TestUi)
#include "ui.moc"

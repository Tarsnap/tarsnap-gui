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
    ui->setupUi(wid);
    delete ui;
    delete wid;
}

QTEST_MAIN(TestUi)
#include "ui.moc"

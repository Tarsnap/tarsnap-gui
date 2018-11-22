#include <QtTest/QtTest>

#include <QFontMetrics>

class TestQTestComplex : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
    void pl_fontmetric_elidedText();
};

void TestQTestComplex::pl_nothing()
{
}

void TestQTestComplex::pl_fontmetric_elidedText()
{
    QFontMetrics metrics(qApp->font());
    QString text = metrics.elidedText("this is a long long long piece of text",
                                      Qt::ElideRight, 100);
}

QTEST_MAIN(TestQTestComplex)
#include "qtest-gui-complex.moc"

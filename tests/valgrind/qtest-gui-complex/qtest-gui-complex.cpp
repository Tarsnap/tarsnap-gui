#include <QtTest/QtTest>

#include <QFileSystemModel>
#include <QFontMetrics>

class TestQTestComplex : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
    void pl_fontmetric_elidedText();
    void pl_processEvents();
    void pl_filesystemmodel();
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

void TestQTestComplex::pl_processEvents()
{
    QCoreApplication::processEvents(0, 100);
}

void TestQTestComplex::pl_filesystemmodel()
{
    QFileSystemModel *model = new QFileSystemModel();

    // Start to read the path (this starts a new thread)
    model->setRootPath(QDir::currentPath());
    QCoreApplication::processEvents(0, 100);

    delete model;
}

QTEST_MAIN(TestQTestComplex)
#include "qtest-gui-complex.moc"

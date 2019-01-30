#include <QtTest/QtTest>

class TestQTestComplex : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();

    // Only the second one triggers a leak
    void pl_app();
    void pl_app_again();
};

void TestQTestComplex::pl_nothing()
{
}

static void make_app()
{
    int   argc = 1;
    char *argv[1];
    argv[0] = strdup("app");

    // Actual memory test
    QCoreApplication *app = new QCoreApplication(argc, argv);
    delete app;

    // Clean up variable
    free(argv[0]);
}

void TestQTestComplex::pl_app()
{
    make_app();
}

void TestQTestComplex::pl_app_again()
{
    make_app();
}

QTEST_MAIN(TestQTestComplex)
#include "qtest-complex.moc"

#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>
WARNINGS_ENABLE

class TestQTestMinimal : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
};

void TestQTestMinimal::pl_nothing()
{
}

QTEST_MAIN(TestQTestMinimal)
#include "qtest-gui-minimal.moc"

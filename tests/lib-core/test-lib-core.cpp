#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QTest>
#include <QVariant>
WARNINGS_ENABLE

#include "TSettings.h"

class TestLibCore : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void settings_default();
    void settings_custom();
    void settings_default_after_custom();
};

void TestLibCore::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);
}

void TestLibCore::cleanupTestCase()
{
    TSettings::destroy();
}

void TestLibCore::settings_default()
{
    // Nuke any previous settings
    TSettings::destroy();

    // Check saved settings
    TSettings settings;
    QString   user = settings.value("tarsnap/user", "").toString();
    QVERIFY(user == "default_init");
}

void TestLibCore::settings_custom()
{
    // Nuke any previous settings
    TSettings::destroy();

    // Set custom filename
    TSettings::setFilename(TEST_DIR "/custom-name.conf");

    // Check saved settings
    TSettings settings;
    QString   user = settings.value("tarsnap/user", "").toString();
    QVERIFY(user == "custom_name");
}

void TestLibCore::settings_default_after_custom()
{
    // Nuke any previous settings
    TSettings::destroy();

    // Nuke a few more times to test that nothing bad happens
    TSettings::destroy();
    TSettings::destroy();

    // Check saved settings -- should be back to default
    TSettings settings;
    QString   user = settings.value("tarsnap/user", "").toString();
    QVERIFY(user == "default_init");
}

QTEST_MAIN(TestLibCore)
WARNINGS_DISABLE
#include "test-lib-core.moc"
WARNINGS_ENABLE

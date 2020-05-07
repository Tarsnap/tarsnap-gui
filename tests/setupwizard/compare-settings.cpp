#include "compare-settings.h"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
WARNINGS_ENABLE

//! Compares all the keys in two QSettings files.
bool compareSettings(QSettings *settings, QSettings *target)
{
    // On OSX, QSettings returns a bunch of values for the system
    // itself (e.g., "com/apple/trackpad/enableSecondaryClick",
    // "NSNavRecentPlaces").  To avoid this, we only examine the
    // groups that are present in the target config file.
    for(int g = 0; g < target->childGroups().length(); g++)
    {
        QString group = target->childGroups().at(g);

        settings->beginGroup(group);
        target->beginGroup(group);

        // Check length of key list
        QStringList settings_keys = settings->allKeys();
        if(settings_keys.length() != target->allKeys().length())
        {
            qDebug() << "compareSettings: number of keys does not match!"
                     << settings_keys.length() << target->allKeys().length();
            return false;
        }

        // Check each key's value
        for(int i = 0; i < settings_keys.length(); i++)
        {
            QString key = settings_keys.at(i);

            // Skip over keys that will be different
            if((group == "tarsnap") && (key == "machine"))
                continue;
#ifdef Q_OS_OSX
            if((group == "tarsnap") && (key == "cache"))
                continue;
            if((group == "app") && (key == "app_data"))
                continue;
#endif

            // Skip over key(s) that can plausibly be different
            if((group == "tarsnap") && (key == "path"))
                continue;

            // Compare values
            if(settings->value(key) != target->value(key))
            {
                qDebug() << "compareSettings: values do not match!" << key
                         << settings->value(key) << target->value(key);
                return false;
            }
        }
        settings->endGroup();
        target->endGroup();
    }

    return true;
}

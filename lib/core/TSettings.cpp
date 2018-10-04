#include <TSettings.h>

#include <QFileInfo>
#include <QMutex>
#include <QSettings>

// Filename to use (if we don't want the default).
static QString filename_next = "";

// The actual settings.
static QSettings *settings = nullptr;

// For safety
static QMutex mutex;

void TSettings::setFilename(QString filename)
{
    filename_next = filename;
}

void TSettings::destroy()
{
    // Bail if we've already cleaned up, including after a mutex (in case a
    // different thread called this function already).
    if(settings == nullptr)
        return;
    mutex.lock();
    if(settings == nullptr)
    {
        mutex.unlock();
        return;
    }

    // Clean up, release mutex.
    delete settings;
    settings      = nullptr;
    filename_next = "";
    mutex.unlock();
}

TSettings::TSettings()
{
    // Bail if we already have an object, including after a mutex (in case a
    // different thread created this object already).
    if(settings != nullptr)
        return;
    mutex.lock();
    if(settings != nullptr)
    {
        mutex.unlock();
        return;
    }

    // If we don't have a custom filename, get the default one for
    // IniFormat, but replace the suffix with ".conf".
    if(filename_next == "")
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings settings_default;
        QFileInfo name(settings_default.fileName());
        // Yes, Qt takes care of "/" for non-Unix platforms.
        filename_next = name.path() + "/" + name.completeBaseName() + ".conf";
    }

    // Create the QSettings.
    settings = new QSettings(filename_next, QSettings::IniFormat);

    // Clean up.
    mutex.unlock();
}

QSettings *TSettings::getQSettings()
{
    return settings;
}

bool TSettings::contains(const QString &key)
{
    return (settings->contains(key));
}

QVariant TSettings::value(const QString &key, const QVariant &defaultValue)
{
    return (settings->value(key, defaultValue));
}

void TSettings::setValue(const QString &key, const QVariant &value)
{
    settings->setValue(key, value);
}

void TSettings::sync()
{
    settings->sync();
}

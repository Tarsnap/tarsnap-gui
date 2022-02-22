#include "TSettings.h"

WARNINGS_DISABLE
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>

// For safety
static QMutex mutex;
WARNINGS_ENABLE

// Initialize static members
QSettings *TSettings::_settings      = nullptr;
QString   *TSettings::_filename_next = nullptr;

void TSettings::setFilename(const QString &filename)
{
    Q_ASSERT(_filename_next == nullptr);

    _filename_next = new QString(filename);
}

void TSettings::destroy()
{
    // Bail if we've already cleaned up, including after a mutex (in case a
    // different thread called this function already).
    if(_settings == nullptr)
        return;
    QMutexLocker locker(&mutex);
    if(_settings == nullptr)
        return;

    // Clean up.
    delete _filename_next;
    delete _settings;
    _settings      = nullptr;
    _filename_next = nullptr;
}

TSettings::TSettings()
{
    // Bail if we already have an object, including after a mutex (in case a
    // different thread created this object already).
    if(_settings != nullptr)
        return;
    QMutexLocker locker(&mutex);
    if(_settings != nullptr)
        return;

    // If we don't have a custom filename, get the default one for
    // IniFormat, but replace the suffix with ".conf".
    if(_filename_next == nullptr)
    {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings _settings_default;
        QFileInfo name(_settings_default.fileName());
        // Yes, Qt takes care of "/" for non-Unix platforms.
        QString path   = name.path() + "/" + name.completeBaseName() + ".conf";
        _filename_next = new QString(path);
    }

    // Create the QSettings.
    _settings = new QSettings(*_filename_next, QSettings::IniFormat);
}

QSettings *TSettings::getQSettings()
{
    return _settings;
}

bool TSettings::contains(const QString &key)
{
    return (_settings->contains(key));
}

QVariant TSettings::value(const QString &key, const QVariant &defaultValue)
{
    return (_settings->value(key, defaultValue));
}

void TSettings::setValue(const QString &key, const QVariant &value)
{
    _settings->setValue(key, value);
}

void TSettings::remove(const QString &key)
{
    _settings->remove(key);
}

void TSettings::clear()
{
    _settings->clear();
}

void TSettings::sync()
{
    _settings->sync();
}

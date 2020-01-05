#include "init-shared.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QList>
#include <QMetaType>
#include <QUrl>
#include <QVector>
WARNINGS_ENABLE

#include "backuptask.h"
#include "debug.h"
#include "notification_info.h"
#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "persistentmodel/journal.h"
#include "persistentmodel/persistentstore.h"
#include "scheduling.h"
#include "tarsnaperror.h"
#include "taskstatus.h"
#include "translator.h"
#include "utils.h"

#include <ConsoleLog.h>
#include <TSettings.h>

#define DEFAULT_LOG_FILE "tarsnap.log"

/*
 * It's unnecessary from a programming standpoint to put these in a separate
 * function, but it helps me keep track of how the various Qt layers work.
 */
static void init_no_app()
{
    qRegisterMetaType<TaskStatus>("TaskStatus");
    qRegisterMetaType<QList<QUrl>>("QList<QUrl>");
    qRegisterMetaType<BackupTaskPtr>("BackupTaskPtr");
    qRegisterMetaType<QList<ArchivePtr>>("QList<ArchivePtr >");
    qRegisterMetaType<ArchivePtr>("ArchivePtr");
    qRegisterMetaType<ArchiveRestoreOptions>("ArchiveRestoreOptions");
    qRegisterMetaType<QSqlQuery>("QSqlQuery");
    qRegisterMetaType<JobPtr>("JobPtr");
    qRegisterMetaType<QMap<QString, JobPtr>>("QMap<QString, JobPtr>");
    qRegisterMetaType<TarsnapError>("TarsnapError");
    qRegisterMetaType<LogEntry>("LogEntry");
    qRegisterMetaType<QVector<LogEntry>>("QVector<LogEntry>");
    qRegisterMetaType<QVector<File>>("QVector<File>");
    qRegisterMetaType<enum message_type>("enum message_type");
}

static void init_no_explicit_app()
{
#ifndef QT_TESTLIB_LIB
    // Don't do this for the tests, since we want them to have unique names.
    QCoreApplication::setOrganizationName(QLatin1String("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(QLatin1String("tarsnap.com"));
    QCoreApplication::setApplicationName(QLatin1String("Tarsnap"));
    QCoreApplication::setApplicationVersion(APP_VERSION);
#endif

#if defined(QT_DEBUG)
    qSetMessagePattern("%{if-debug}%{file}(%{line}): %{endif}%{message}");
#endif

    // In order to avoid a memory leak (?), must be done after setting up
    // the application and/or organization name.
    ConsoleLog::initializeConsoleLog();
}

/**
 * Constructor initialization shared between GUI and non-GUI.  Cannot fail.
 */
void init_shared()
{
    init_no_app();
    init_no_explicit_app();

    QCoreApplication::setQuitLockEnabled(false);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
}

static QString migrateSettings(QSettings *settingsOld, QSettings *settingsNew)
{
    // Copy old settings to new, by group.  (On OSX, QSettings contains a whole
    // bunch of system-wide settings which we don't want to copy.)
    QStringList groups = {"app", "tarsnap"};
    for(const QString &groupname : groups)
    {
        settingsOld->beginGroup(groupname);
        settingsNew->beginGroup(groupname);
        QStringList keys = settingsOld->childKeys();
        for(const QString &key : keys)
        {
            settingsNew->setValue(key, settingsOld->value(key));
        }
        settingsOld->endGroup();
        settingsNew->endGroup();
    }
    settingsNew->sync();

    // Rename old settings to prevent migrating it again.
    QFile   fileOld(settingsOld->fileName());
    QString renamed = settingsOld->fileName()
                      + QDate::currentDate().toString(".yyyy-MMM-dd.bak");

    // Close the Setings to prevent it from re-writing the file.
    delete settingsOld;
    fileOld.rename(renamed);
    return renamed;
}

static QString check_migrateSettings()
{
    // Get default settings file.
    QSettings::setDefaultFormat(QSettings::NativeFormat);
    QSettings *settingsOld = new QSettings();

    // Shouldn't be necessary, but just in case.
    TSettings::destroy();

    // Get new settings file.  Must be done after getting the default one!
    TSettings  tsettings;
    QSettings *settingsNew = tsettings.getQSettings();

    // Bail if we don't need to migrate anything.
    if(QFileInfo::exists(settingsNew->fileName())
       || !QFileInfo::exists(settingsOld->fileName()))
    {
        delete settingsOld;
        return "";
    }

    return migrateSettings(settingsOld, settingsNew);
}

/**
 * Configures the app-wide Settings.  Can fail and report messages.
 */
struct init_info init_shared_settings(QString configDir)
{
    struct init_info info = {INIT_OK, "", ""};

    // Handle soon-to-be-deprecated --appdata argument.  Must be done before
    // instantiating any settings object.
    if(!configDir.isEmpty())
    {
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                           configDir);
        QSettings::setDefaultFormat(QSettings::IniFormat);
        // Make sure that TSettings uses the right filename in this case.
        QSettings defaultSettings;
        TSettings::setFilename(defaultSettings.fileName());
    }

    // Migrate settings from old settings file to new (if applicable).
    QString renamedOldSettings;
    if(configDir.isEmpty())
    {
        renamedOldSettings = check_migrateSettings();
    }

    TSettings settings;

    if(!renamedOldSettings.isEmpty())
    {
        info.status = INIT_SETTINGS_RENAMED;
        info.message =
            QString("Updated config file, new location:\n\n%1\n\nThe old file "
                    "was renamed to:\n\n%2")
                .arg(settings.getQSettings()->fileName(), renamedOldSettings);
    }

    return info;
}

static QString getDefaultLogFilename()
{
    TSettings settings;
    QString   appdata = settings.value("app/app_data", "").toString();
    if(appdata.isEmpty())
    {
        DEBUG << "Error saving Console Log message: app/app_data dir not set.";
        return "";
    }

    return appdata + QDir::separator() + DEFAULT_LOG_FILE;
}

/**
 * Initialization shared between GUI and non-GUI.  Can fail and report messages.
 */
struct init_info init_shared_core(QCoreApplication *app)
{
    struct init_info info = {INIT_OK, "", ""};
    TSettings        settings;

    // Set up the translator.
    Translator::initializeTranslator();
    Translator &translator = Translator::instance();
    translator.translateApp(
        app, settings.value("app/language", LANG_AUTO).toString());

    // Run the setup wizard (if necessary).  This uses the translator, and
    // can be tested with:
    //    $ LANGUAGE=ro ./tarsnap-gui
    bool wizardDone = settings.value("app/wizard_done", false).toBool();
    if(!wizardDone)
    {
        info.status = INIT_NEEDS_SETUP;
        return (info);
    }

    // Ensure that we have a location to store app data.  Must be
    // after the setup wizard!
    if(settings.value("app/app_data", "").toString().isEmpty())
    {
        QString appDataDir = QStandardPaths::writableLocation(APPDATA);
        LOG << "Setting default \"app/app_data\" to" << appDataDir;
        settings.setValue("app/app_data", appDataDir);
    }

    // Set up the log file.  Must be done after setup wizard!
    LOG.setFilename(getDefaultLogFilename());

    // Initialize the persistentstore.  Must be after setup wizard!
    PersistentStore::initializePersistentStore();
    PersistentStore &store = PersistentStore::instance();
    if(!store.init())
    {
        info.status = INIT_DB_FAILED;
        return (info);
    }

    // Warn about --dry-run before trying to run --jobs.
    if(settings.value("tarsnap/dry_run", false).toBool())
    {
        info.status  = INIT_DRY_RUN;
        info.message = QObject::tr("Simulation mode is enabled.  Archives will"
                                   " not be uploaded to the Tarsnap server."
                                   "  Disable in Settings -> Backup.");
        return (info);
    }

    // Make sure we have the path to the current Tarsnap-GUI binary
    struct scheduleinfo correctedPath = correctedSchedulingPath();

    if(correctedPath.status == SCHEDULE_OK)
    {
        info.status  = INIT_SCHEDULE_OK;
        info.message = correctedPath.message;
    }
    if(correctedPath.status == SCHEDULE_ERROR)
    {
        info.status  = INIT_SCHEDULE_ERROR;
        info.message = correctedPath.message;
        info.extra   = correctedPath.extra;
    }

    return info;
}

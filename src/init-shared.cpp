#include "init-shared.h"

#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "persistentmodel/journal.h"
#include "scheduling.h"
#include "tarsnaperror.h"
#include "taskstatus.h"
#include "translator.h"

#include <QCoreApplication>
#include <QFile>
#include <QList>
#include <QMetaType>
#include <QUrl>
#include <QVector>

#include <TSettings.h>

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
}

/**
 * Constructor initialization shared between GUI and non-GUI.  Cannot fail.
 */
void init_shared(QCoreApplication *app)
{
    init_no_app();
    init_no_explicit_app();

    app->setQuitLockEnabled(false);
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
}

static QString migrateSettings(QSettings *settingsOld, QSettings *settingsNew)
{
    // Copy old settings to new, by group.  (On OSX, QSettings contains a whole
    // bunch of system-wide settings which we don't want to copy.)
    QStringList groups = {"app", "tarsnap"};
    for(QStringList::iterator g = groups.begin(); g != groups.end(); g++)
    {
        settingsOld->beginGroup(*g);
        settingsNew->beginGroup(*g);
        QStringList keys = settingsOld->childKeys();
        for(QStringList::iterator i = keys.begin(); i != keys.end(); i++)
        {
            settingsNew->setValue(*i, settingsOld->value(*i));
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

/**
 * Initialization shared between GUI and non-GUI.  Can fail and report messages.
 */
struct init_info init_shared_core(QCoreApplication *app)
{
    struct init_info info = {INIT_OK, "", ""};
    TSettings        settings;

    // Set up the translator.
    Translator &translator = Translator::instance();
    translator.translateApp(app,
                            settings.value("app/language", LANG_AUTO).toString());

    // Run the setup wizard (if necessary).  This uses the translator, and
    // can be tested with:
    //    $ LANGUAGE=ro ./tarsnap-gui
    bool wizardDone = settings.value("app/wizard_done", false).toBool();
    if(!wizardDone)
    {
        info.status = INIT_NEEDS_SETUP;
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

#include "init-shared.h"

#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"
#include "persistentmodel/journal.h"
#include "tarsnaperror.h"
#include "taskstatus.h"

#include <QCoreApplication>
#include <QFile>
#include <QList>
#include <QMetaType>
#include <QUrl>
#include <QVector>

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
    QCoreApplication::setOrganizationName(QLatin1String("Tarsnap Backup Inc."));
    QCoreApplication::setOrganizationDomain(QLatin1String("tarsnap.com"));
    QCoreApplication::setApplicationName(QLatin1String("Tarsnap"));
    QCoreApplication::setApplicationVersion(APP_VERSION);
}

void init_shared(QCoreApplication *app)
{
    init_no_app();
    init_no_explicit_app();

    app->setQuitLockEnabled(false);
    app->setAttribute(Qt::AA_UseHighDpiPixmaps);
}

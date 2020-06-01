#include "scheduling.h"

WARNINGS_DISABLE
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QLatin1String>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegExp>
#include <QString>
#include <QStringList>

#if defined(Q_OS_OSX)
#include <QSettings>
#endif

WARNINGS_ENABLE

#include "debug.h"

#if defined(Q_OS_OSX)
#define UPDATED_LAUNCHD_PATH_LONG                                              \
    "The OS X launchd scheduling service contained an out-of-date link to "    \
    "Tarsnap GUI (did you upgrade it recently?).\n\nThis has been updated to " \
    "point to the current Tarsnap GUI."

#define UPDATED_LAUNCHD_PATH_SHORT "Updated launchd path to Tarsnap GUI"

#define UPDATED_LAUNCHD_PATH_ERROR                                             \
    "An error occurred while attempting to update the OS X launchd path."
#endif

#define CRON_LINE "*/30 * * * * /usr/bin/env %1 %2 %3 %4 --jobs"
#define CRON_MARKER_BEGIN "#BEGIN Tarsnap GUI - DO NOT EDIT THIS BLOCK MANUALLY"
#define CRON_MARKER_END "#END Tarsnap GUI - DO NOT EDIT THIS BLOCK MANUALLY"
#define CRON_MARKER_HELP                                                       \
    "# For help, see: https://github.com/Tarsnap/tarsnap-gui/wiki/Scheduling"

//! Information about running a command.
struct cmdinfo
{
    //! Exit code of the command.
    int exit_code;
    //! Standard error from the command.
    QByteArray stderr_msg;
    //! Standard output from the command.
    QByteArray stdout_msg;
};

static struct cmdinfo runCmd(const QString &cmd, const QStringList &args,
                             const QByteArray *stdin_msg = nullptr)
{
    QProcess       proc;
    struct cmdinfo info;
    proc.start(cmd, args);

    // We can want stdin_msg to be empty but non-null; for example if we're
    // disabling the scheduling and therefore writing an empty crontab file.
    if(stdin_msg != nullptr)
    {
        proc.write(*stdin_msg);
        proc.closeWriteChannel();
    }
    proc.waitForFinished(-1);

    // Get exit code, working around QProcess not having a valid exitCode()
    // if there's a crash.
    info.exit_code = proc.exitCode();
    if(proc.exitStatus() != QProcess::NormalExit)
        info.exit_code = -1;

    info.stderr_msg = proc.readAllStandardError();
    info.stdout_msg = proc.readAllStandardOutput();

    return (info);
}

// This is an awkward hack which is an intermediate step towards separating
// the front-end and back-end code.  Return values:
//   0: everything ok
//   1: failed to load
//   2: failed to start
static int launchdLoad()
{
    struct cmdinfo pinfo;
    QString        launchdPlistFileName =
        QDir::homePath() + "/Library/LaunchAgents/com.tarsnap.gui.plist";

    pinfo =
        runCmd("launchctl", QStringList() << "load" << launchdPlistFileName);
    if(pinfo.exit_code != 0)
        return (1);

    pinfo = runCmd("launchctl", QStringList() << "start"
                                              << "com.tarsnap.gui");
    if(pinfo.exit_code != 0)
        return (2);

    return (0);
}

// Return values:
//   0: everything ok
//   1: failed to unload
static int launchdUnload()
{
    struct cmdinfo pinfo;
    QString        launchdPlistFileName =
        QDir::homePath() + "/Library/LaunchAgents/com.tarsnap.gui.plist";

    pinfo =
        runCmd("launchctl", QStringList() << "unload" << launchdPlistFileName);
    if(pinfo.exit_code != 0)
        return (1);

    return (0);
}

#if defined(Q_OS_OSX)
static bool launchdLoaded()
{
    struct cmdinfo pinfo;

    pinfo = runCmd("launchctl", QStringList() << "list"
                                              << "com.tarsnap.gui");
    if(pinfo.exit_code != 0)
        return (false);

    return (true);
}
#endif

struct scheduleinfo launchdEnable()
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};

    QFile launchdPlist(":/com.tarsnap.gui.plist");
    launchdPlist.open(QIODevice::ReadOnly | QIODevice::Text);
    QFile launchdPlistFile(QDir::homePath()
                           + "/Library/LaunchAgents/com.tarsnap.gui.plist");
    if(launchdPlistFile.exists())
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr("Looks like scheduling is already enabled."
                                   " Nothing to do.\n\n%1")
                           .arg(CRON_MARKER_HELP);
        return info;
    }
    if(!launchdPlistFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        info.status = SCHEDULE_ERROR;
        info.message =
            QObject::tr("Failed to write service file %1. Aborting operation.")
                .arg(launchdPlistFile.fileName());
        return info;
    }
    launchdPlistFile.write(
        launchdPlist.readAll()
            .replace("%1", QCoreApplication::applicationFilePath().toLatin1())
            .replace("%2", QDir::homePath().toLatin1()));
    launchdPlist.close();
    launchdPlistFile.close();

    int ret = launchdLoad();
    if(ret == 1)
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr("Failed to load launchd service file.");
        return info;
    }
    else if(ret == 2)
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr("Failed to start launchd service file.");
        return info;
    }
    return info;
}

struct scheduleinfo launchdDisable()
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};

    QFile launchdPlistFile(QDir::homePath()
                           + "/Library/LaunchAgents/com.tarsnap.gui.plist");
    if(!launchdPlistFile.exists())
    {
        info.status = SCHEDULE_ERROR;
        info.message =
            QObject::tr("Launchd service file not found:\n%1\n Nothing to do.")
                .arg(launchdPlistFile.fileName());
        return info;
    }

    int ret = launchdUnload();
    if(ret == 1)
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr("Failed to unload launchd service.");
        return info;
    }

    if(!launchdPlistFile.remove())
    {
        info.status = SCHEDULE_ERROR;
        info.message =
            QObject::tr("Cannot remove service file:\n%1\nAborting operation.")
                .arg(launchdPlistFile.fileName());
        return info;
    }
    return info;
}

struct scheduleinfo cronEnable()
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};

    struct cmdinfo pinfo;
    pinfo = runCmd("crontab", QStringList() << "-l");
    if(pinfo.exit_code != 0)
    {
        QString error(pinfo.stderr_msg);
        /* On some distros crontab -l exits with error 1 and message
         * "no crontab for username" if there's no crontab installed
         * for the current user. If parent is the case proceed and don't err.
         */
        if(-1 == error.indexOf(QRegExp("^(crontab: )?no crontab for")))
        {
            info.status = SCHEDULE_ERROR;
            info.message =
                QObject::tr("Failed to list current crontab: %1").arg(error);
            return info;
        }
    }
    QString currentCrontab(pinfo.stdout_msg);

    QRegExp rx(QString("\n?%1.+%2\n?")
                   .arg(QRegExp::escape(CRON_MARKER_BEGIN))
                   .arg(QRegExp::escape(CRON_MARKER_END)));
    rx.setMinimal(true);
    if(-1 != rx.indexIn(currentCrontab))
    {
        info.status = SCHEDULE_ERROR;
        info.message =
            QObject::tr("Looks like scheduling is already enabled for the"
                        " current user's crontab. Nothing to do."
                        "\n%1")
                .arg(CRON_MARKER_HELP);
        return info;
    }

    QString             cronLine(CRON_LINE);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    cronLine =
        cronLine
            .arg(env.contains("SCREEN") ? "SCREEN=" + env.value("SCREEN") : "")
            .arg(env.contains("DISPLAY") ? "DISPLAY=" + env.value("DISPLAY")
                                         : "")
            .arg(env.contains("XAUTHORITY")
                     ? "XAUTHORITY=" + env.value("XAUTHORITY")
                     : "")
            .arg(QCoreApplication::applicationFilePath());

    QString cronBlock("\n%1\n%2\n%3\n%4\n");
    cronBlock = cronBlock.arg(CRON_MARKER_BEGIN)
                    .arg(CRON_MARKER_HELP)
                    .arg(cronLine)
                    .arg(CRON_MARKER_END);

    info.status  = SCHEDULE_NEED_INFO;
    info.message = cronBlock;
    info.extra   = currentCrontab;

    return info;
}

struct scheduleinfo cronEnable_p2(const QString &cronBlock,
                                  const QString &existingCrontab)
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};
    struct cmdinfo      pinfo;
    QString             currentCrontab(existingCrontab);

    currentCrontab.append(cronBlock.toLatin1());
    QByteArray newCrontab = currentCrontab.toLatin1();

    pinfo = runCmd("crontab", QStringList() << "-", &newCrontab);
    if(pinfo.exit_code != 0)
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr("Failed to update crontab: %1")
                           .arg(QString(pinfo.stderr_msg));
        return info;
    }
    return info;
}

struct scheduleinfo cronDisable()
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};
    struct cmdinfo      pinfo;

    pinfo = runCmd("crontab", QStringList() << "-l");
    if(pinfo.exit_code != 0)
    {
        QString error(pinfo.stderr_msg);
        /* On some distros crontab -l exits with error 1 and message
         * "no crontab for username" if there's no crontab installed
         * for the current user. If parent is the case proceed and don't err.
         */
        if(error.startsWith(QLatin1String("no crontab for")))
        {
            info.status = SCHEDULE_ERROR;
            info.message =
                QObject::tr("There's no crontab for the current user."
                            " Nothing to do.\n\n%1")
                    .arg(CRON_MARKER_HELP);
            return info;
        }
        else
        {
            info.status = SCHEDULE_ERROR;
            info.message =
                QObject::tr("Failed to list current crontab: %1").arg(error);
            return info;
        }
    }
    QString currentCrontab(pinfo.stdout_msg);
    if(currentCrontab.isEmpty())
    {
        info.status = SCHEDULE_ERROR;
        info.message =
            QObject::tr("Looks like the crontab for the current user is"
                        " empty. Nothing to do.\n\n%1")
                .arg(CRON_MARKER_HELP);
        return info;
    }

    QRegExp rx(QString("\n?%1.+%2\n?")
                   .arg(QRegExp::escape(CRON_MARKER_BEGIN))
                   .arg(QRegExp::escape(CRON_MARKER_END)));
    //    rx.setMinimal(true);
    QString linesToRemove;
    int     pos = 0;
    while((pos = rx.indexIn(currentCrontab, pos)) != -1)
    {
        linesToRemove += rx.cap();
        pos += rx.matchedLength();
    }

    if(linesToRemove.isEmpty())
    {
        info.status = SCHEDULE_ERROR;
        info.message =
            QObject::tr("Looks like Job scheduling hasn't been enabled"
                        " yet. Nothing to do. \n\n%1")
                .arg(CRON_MARKER_HELP);
        return info;
    }
    info.status  = SCHEDULE_NEED_INFO;
    info.message = linesToRemove;
    info.extra   = currentCrontab;
    return info;
}

struct scheduleinfo cronDisable_p2(const QString &linesToRemove,
                                   const QString &existingCrontab)
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};
    struct cmdinfo      pinfo;
    QString             currentCrontab(existingCrontab);

    currentCrontab.remove(linesToRemove);
    DEBUG << currentCrontab;
    QByteArray newCrontab = currentCrontab.toLatin1();

    pinfo = runCmd("crontab", QStringList() << "-", &newCrontab);
    if(pinfo.exit_code != 0)
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr("Failed to update crontab: %1")
                           .arg(QString(pinfo.stderr_msg));
        return info;
    }
    return info;
}

struct scheduleinfo correctedSchedulingPath()
{
    struct scheduleinfo info = {SCHEDULE_NOTHING_HAPPENED, "", ""};
#if defined(Q_OS_OSX)
    QSettings launchdPlist(QDir::homePath()
                               + "/Library/LaunchAgents/com.tarsnap.gui.plist",
                           QSettings::NativeFormat);

    // Bail if the file doesn't exist, but this isn't an error.
    if(!launchdPlist.contains("ProgramArguments"))
        return (info);

    // Get path, bail if it still exists (we assume it's still executable)
    QStringList args =
        launchdPlist.value("ProgramArguments").value<QStringList>();
    if(QFile::exists(args.at(0)))
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr(UPDATED_LAUNCHD_PATH_ERROR);
        return (info);
    }

    // Update the path
    args.replace(0, QCoreApplication::applicationFilePath().toLatin1());
    launchdPlist.setValue("ProgramArguments", args);
    launchdPlist.sync();

    // Stop launchd script if it's loaded
    if(launchdLoaded())
    {
        if(launchdUnload() != 0)
        {
            info.status  = SCHEDULE_ERROR;
            info.message = QObject::tr(UPDATED_LAUNCHD_PATH_ERROR);
            return (info);
        }
    }

    // Load (and start) new program
    if(launchdLoad() != 0)
    {
        info.status  = SCHEDULE_ERROR;
        info.message = QObject::tr(UPDATED_LAUNCHD_PATH_ERROR);
        return (info);
    }

    info.status  = SCHEDULE_OK;
    info.message = QObject::tr(UPDATED_LAUNCHD_PATH_LONG);
    info.extra   = QObject::tr(UPDATED_LAUNCHD_PATH_SHORT);
    return (info);
#else
    return (info);
#endif
}

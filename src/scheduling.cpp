#include <QAbstractButton>
#include <QCoreApplication>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QString>

#include "debug.h"
#include "utils.h"

#include "scheduling.h"

struct cmdinfo
{
    int        exit_code;
    QByteArray stderr_msg;
    QByteArray stdout_msg;
};

enum schedulestatus
{
    SCHEDULE_OK,
    SCHEDULE_ERROR,
    SCHEDULE_NEED_INFO
};

struct scheduleinfo
{
    schedulestatus status;
    QString        message;
    QString        extra;
};

static struct cmdinfo runCmd(QString cmd, QStringList args,
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

#if defined(Q_OS_OSX)
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

    pinfo = runCmd("launchctl", QStringList() << "load" << launchdPlistFileName);
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

static bool launchdLoaded()
{
    struct cmdinfo pinfo;

    pinfo = runCmd("launchctl", QStringList() << "list"
                                              << "com.tarsnap.gui");
    if(pinfo.exit_code != 0)
        return (false);

    return (true);
}

static struct scheduleinfo launchdEnable()
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

static struct scheduleinfo launchdDisable()
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

#endif

static struct scheduleinfo cronEnable()
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

static struct scheduleinfo cronEnable_p2(QString cronBlock,
                                         QString currentCrontab)
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};
    struct cmdinfo      pinfo;

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

static struct scheduleinfo cronDisable()
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

static struct scheduleinfo cronDisable_p2(QString linesToRemove,
                                          QString currentCrontab)
{
    struct scheduleinfo info = {SCHEDULE_OK, "", ""};
    struct cmdinfo      pinfo;

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

Scheduling::Scheduling(QWidget *parent_new)
    : QObject(parent_new), parent(parent_new)
{
}

Scheduling::~Scheduling()
{
}

void Scheduling::enableJobScheduling()
{
#if defined(Q_OS_OSX)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(parent, tr("Job scheduling"),
                              tr("Register Tarsnap GUI with the OS X"
                                 " Launchd service to run daily at 10am?"
                                 "\n\nJobs that have scheduled backup"
                                 " turned on will be backed up according"
                                 " to the Daily, Weekly or Monthly"
                                 " schedule. \n\n%1")
                                  .arg(CRON_MARKER_HELP));
    if(confirm != QMessageBox::Yes)
        return;

    struct info = launchdEnable();
    if(info.status != SCHEDULE_OK)
    {
        QMessageBox::critical(parent, tr("Job scheduling"), status.message);
        return;
    }

#elif defined(Q_OS_LINUX) || defined(Q_OS_BSD4)

    QMessageBox::StandardButton confirm =
        QMessageBox::question(parent, tr("Job scheduling"),
                              tr("Register Tarsnap GUI with cron serivce?"
                                 "\nJobs that have scheduled backup"
                                 " turned on will be backed up according"
                                 " to the Daily, Weekly or Monthly"
                                 " schedule. \n\n%1")
                                  .arg(CRON_MARKER_HELP));
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = cronEnable();
    if(info.status == SCHEDULE_ERROR)
    {
        QMessageBox::critical(parent, tr("Job scheduling"), info.message);
        return;
    }
    else if(info.status == SCHEDULE_OK)
    {
        QMessageBox::critical(parent, tr("Job scheduling"),
                              "Unknown error in scheduling code.");
        return;
    }
    QString cronBlock = info.message;

    QMessageBox question(parent);
    question.setIcon(QMessageBox::Question);
    question.setText(QObject::tr(
        "Tarsnap GUI will be added to the current user's crontab."));
    question.setInformativeText(
        QObject::tr("To ensure proper behavior please review the"
                    " lines to be added by pressing Show"
                    " Details before proceeding."));
    question.setDetailedText(cronBlock);
    question.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    question.setDefaultButton(QMessageBox::Cancel);
    // Workaround for activating Show details by default
    foreach(QAbstractButton *button, question.buttons())
    {
        if(question.buttonRole(button) == QMessageBox::ActionRole)
        {
            button->click();
            break;
        }
    }
    int proceed = question.exec();
    if(proceed == QMessageBox::Cancel)
        return;

    struct scheduleinfo info_p2 = cronEnable_p2(cronBlock, info.extra);
    if(info_p2.status != SCHEDULE_OK)
    {
        QMessageBox::critical(parent, tr("Job scheduling"), info.message);
        return;
    }
#endif
}

void Scheduling::disableJobScheduling()
{
#if defined(Q_OS_OSX)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(parent, tr("Job scheduling"),
                              tr("Unregister Tarsnap GUI from the OS X"
                                 " Launchd service? This will disable"
                                 " automatic Job backup scheduling."
                                 "\n\n%1")
                                  .arg(CRON_MARKER_HELP));
    if(confirm != QMessageBox::Yes)
        return;

    struct info = launchdDisable();
    if(info.status != SCHEDULE_OK)
    {
        QMessageBox::critical(parent, tr("Job scheduling"), status.message);
        return;
    }

#elif defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(parent, "Confirm action",
                              "Unregister Tarsnap GUI from cron?");
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = cronDisable();
    if(info.status == SCHEDULE_ERROR)
    {
        QMessageBox::critical(parent, tr("Job scheduling"), info.message);
        return;
    }
    else if(info.status == SCHEDULE_OK)
    {
        QMessageBox::critical(parent, tr("Job scheduling"),
                              "Unknown error in scheduling code.");
        return;
    }
    QString linesToRemove = info.message;

    QMessageBox question(parent);
    question.setIcon(QMessageBox::Question);
    question.setText(tr("Tarsnap GUI will be removed from the current user's"
                        " crontab."));
    question.setInformativeText(
        tr("To ensure proper behavior please review the"
           " lines to be removed by pressing Show Details"
           " before proceeding."));
    question.setDetailedText(linesToRemove);
    question.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    question.setDefaultButton(QMessageBox::Cancel);
    // Workaround for activating Show details by default
    foreach(QAbstractButton *button, question.buttons())
    {
        if(question.buttonRole(button) == QMessageBox::ActionRole)
        {
            button->click();
            break;
        }
    }
    int proceed = question.exec();
    if(proceed == QMessageBox::Cancel)
        return;

    struct scheduleinfo info_p2 = cronDisable_p2(linesToRemove, info.extra);
    if(info_p2.status != SCHEDULE_OK)
    {
        QMessageBox::critical(parent, tr("Job scheduling"), info.message);
        return;
    }
#endif
}

// Returns:
//     -1: no change
//     0: changed successfully
//     1: an error occurred
int Scheduling::correctedSchedulingPath()
{
#if defined(Q_OS_OSX)
    QSettings launchdPlist(QDir::homePath()
                               + "/Library/LaunchAgents/com.tarsnap.gui.plist",
                           QSettings::NativeFormat);

    // Bail if the file doesn't exist
    if(!launchdPlist.contains("ProgramArguments"))
        return (-1);

    // Get path, bail if it still exists (we assume it's still executable)
    QStringList args =
        launchdPlist.value("ProgramArguments").value<QStringList>();
    if(QFile::exists(args.at(0)))
        return (-1);

    // Update the path
    args.replace(0, QCoreApplication::applicationFilePath().toLatin1());
    launchdPlist.setValue("ProgramArguments", args);
    launchdPlist.sync();

    // Stop launchd script if it's loaded
    if(launchdLoaded())
    {
        if(launchdUnload() != 0)
            return (1);
    }

    // Load (and start) new program
    if(launchdLoad() != 0)
        return (1);

    return (0);
#else
    return (-1);
#endif
}

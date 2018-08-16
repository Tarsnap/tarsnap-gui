#include <QAbstractButton>
#include <QCoreApplication>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
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
#endif

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

    QFile launchdPlist(":/com.tarsnap.gui.plist");
    launchdPlist.open(QIODevice::ReadOnly | QIODevice::Text);
    QFile launchdPlistFile(QDir::homePath()
                           + "/Library/LaunchAgents/com.tarsnap.gui.plist");
    if(launchdPlistFile.exists())
    {
        QMessageBox::critical(parent, tr("Job scheduling"),
                              tr("Looks like scheduling is already enabled."
                                 " Nothing to do.\n\n%1")
                                  .arg(CRON_MARKER_HELP));
        return;
    }
    if(!launchdPlistFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QString msg(tr("Failed to write service file %1. Aborting operation."));
        msg = msg.arg(launchdPlistFile.fileName());
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
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
        QString msg(tr("Failed to load launchd service file."));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
    }
    else if(ret == 2)
    {
        QString msg(tr("Failed to start launchd service."));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
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
            QString msg(tr("Failed to list current crontab: %1"));
            msg = msg.arg(error);
            DEBUG << msg;
            QMessageBox::critical(parent, tr("Job scheduling"), msg);
            return;
        }
    }
    QByteArray currentCrontab = pinfo.stdout_msg;

    QRegExp rx(QString("\n?%1.+%2\n?")
                   .arg(QRegExp::escape(CRON_MARKER_BEGIN))
                   .arg(QRegExp::escape(CRON_MARKER_END)));
    rx.setMinimal(true);
    if(-1 != rx.indexIn(currentCrontab))
    {
        QMessageBox::critical(
            parent, tr("Job scheduling"),
            tr("Looks like scheduling is already enabled for the"
               " current user's crontab. Nothing to do."
               "\n%1")
                .arg(CRON_MARKER_HELP));
        return;
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

    QMessageBox question(parent);
    question.setIcon(QMessageBox::Question);
    question.setText(
        tr("Tarsnap GUI will be added to the current user's crontab."));
    question.setInformativeText(tr("To ensure proper behavior please review the"
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

    currentCrontab.append(cronBlock.toLatin1());
    DEBUG << currentCrontab;
    QByteArray newCrontab = currentCrontab;

    pinfo = runCmd("crontab", QStringList() << "-", &newCrontab);
    if(pinfo.exit_code != 0)
    {
        QString msg(tr("Failed to update crontab: %1"));
        msg = msg.arg(QString(pinfo.stderr_msg));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
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

    QFile launchdPlistFile(QDir::homePath()
                           + "/Library/LaunchAgents/com.tarsnap.gui.plist");
    if(!launchdPlistFile.exists())
    {
        QString msg(tr("Launchd service file not found:\n%1\n Nothing to do."));
        msg = msg.arg(launchdPlistFile.fileName());
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
    }

    int ret = launchdUnload();
    if(ret == 1)
    {
        QString msg(tr("Failed to unload launchd service."));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
    }

    if(!launchdPlistFile.remove())
    {
        QString msg(tr("Cannot remove service file:\n%1\nAborting operation."));
        msg = msg.arg(launchdPlistFile.fileName());
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(parent, "Confirm action",
                              "Unregister Tarsnap GUI from cron?");
    if(confirm != QMessageBox::Yes)
        return;

    struct cmdinfo pinfo;
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
            QMessageBox::warning(parent, tr("Job scheduling"),
                                 tr("There's no crontab for the current user."
                                    " Nothing to do.\n\n%1")
                                     .arg(CRON_MARKER_HELP));
            return;
        }
        else
        {
            QString msg(tr("Failed to list current crontab: %1"));
            msg = msg.arg(error);
            DEBUG << msg;
            QMessageBox::critical(parent, tr("Job scheduling"), msg);
            return;
        }
    }
    QString currentCrontab(pinfo.stdout_msg);
    if(currentCrontab.isEmpty())
    {
        QMessageBox::warning(parent, tr("Job scheduling"),
                             tr("Looks like the crontab for the current user is"
                                " empty. Nothing to do.\n\n%1")
                                 .arg(CRON_MARKER_HELP));
        return;
    }

    DEBUG << currentCrontab;
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
        QMessageBox::warning(parent, tr("Job scheduling"),
                             tr("Looks like Job scheduling hasn't been enabled"
                                " yet. Nothing to do. \n\n%1")
                                 .arg(CRON_MARKER_HELP));
        return;
    }

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

    currentCrontab.remove(linesToRemove);
    DEBUG << currentCrontab;
    QByteArray newCrontab = currentCrontab.toLatin1();

    pinfo = runCmd("crontab", QStringList() << "-", &newCrontab);
    if(pinfo.exit_code != 0)
    {
        QString msg(tr("Failed to update crontab: %1"));
        msg = msg.arg(QString(pinfo.stderr_msg));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
    }
#endif
}

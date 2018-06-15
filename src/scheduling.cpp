#include <QAbstractButton>
#include <QCoreApplication>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QString>

#include "debug.h"
#include "utils.h"

#include "scheduling.h"

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

    QProcess launchctl;
    launchctl.start("launchctl", QStringList() << "load"
                                               << launchdPlistFile.fileName());
    launchctl.waitForFinished(-1);
    if((launchctl.exitStatus() != QProcess::NormalExit)
       || (launchctl.exitCode() != 0))
    {
        QString msg(tr("Failed to load launchd service file."));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
    }

    launchctl.start("launchctl", QStringList() << "start"
                                               << "com.tarsnap.gui");
    launchctl.waitForFinished(-1);
    if((launchctl.exitStatus() != QProcess::NormalExit)
       || (launchctl.exitCode() != 0))
    {
        QString msg(tr("Failed to start launchd service."));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
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

    QProcess crontab;
    crontab.start("crontab", QStringList() << "-l");
    crontab.waitForFinished(-1);
    if((crontab.exitStatus() != QProcess::NormalExit)
       || (crontab.exitCode() != 0))
    {
        QString error(crontab.readAllStandardError());
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
    QByteArray currentCrontab = crontab.readAllStandardOutput();

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

    crontab.start("crontab", QStringList() << "-");
    crontab.write(currentCrontab);
    crontab.closeWriteChannel();
    crontab.waitForFinished(-1);
    if((crontab.exitStatus() != QProcess::NormalExit)
       || (crontab.exitCode() != 0))
    {
        QString msg(tr("Failed to update crontab: %1"));
        msg = msg.arg(QString(crontab.readAllStandardError()));
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

    QProcess process;
    process.start("launchctl", QStringList() << "unload"
                                             << launchdPlistFile.fileName());
    process.waitForFinished(-1);
    if((process.exitStatus() != QProcess::NormalExit)
       || (process.exitCode() != 0))
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

    QProcess crontab;
    crontab.start("crontab", QStringList() << "-l");
    crontab.waitForFinished(-1);
    if((crontab.exitStatus() != QProcess::NormalExit)
       || (crontab.exitCode() != 0))
    {
        QString error(crontab.readAllStandardError());
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
    QString currentCrontab(crontab.readAllStandardOutput());
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

    crontab.start("crontab", QStringList() << "-");
    crontab.write(currentCrontab.toLatin1());
    crontab.closeWriteChannel();
    crontab.waitForFinished(-1);
    if((crontab.exitStatus() != QProcess::NormalExit)
       || (crontab.exitCode() != 0))
    {
        QString msg(tr("Failed to update crontab: %1"));
        msg = msg.arg(QString(crontab.readAllStandardError()));
        DEBUG << msg;
        QMessageBox::critical(parent, tr("Job scheduling"), msg);
        return;
    }
#endif
}

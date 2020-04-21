#include "jobrunner.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDate>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QVariant>
WARNINGS_ENABLE

#include "TSettings.h"

#include "messages/notification_info.h"

#include "backuptask.h"
#include "debug.h"
#include "persistentmodel/job.h"
#include "tasks/tasks-defs.h"

JobRunner::JobRunner()
{
}

bool JobRunner::waitForOnline()
{
    for(int i = 0; i < NETWORK_UP_ATTEMPTS; i++)
    {
        QTcpSocket sock;
        sock.connectToHost(NETWORK_HOST, NETWORK_PORT);
        // Multiple calls to waitForConnected() do not accumulate time, so we
        // use ::sleep() below.
        if(sock.waitForConnected(500))
        {
            sock.disconnectFromHost();
            return true;
        }
        // Wait a bit, then try again
        if(i < NETWORK_UP_ATTEMPTS - 1)
            QThread::sleep(NETWORK_UP_SLEEP);
    }
    warnNotOnline();
    return false;
}

void JobRunner::warnNotOnline()
{
    QString title(tr("Scheduled jobs not executed (cannot reach server)."));
    QString body(tr("Please check your internet connectivity and try again."));

    emit message(title, body);
    emit displayNotification(title + "\n" + body, NOTIFICATION_NOT_ONLINE, "");
    // Quit with a delay to allow for the system notifications to go through
    QTimer *quitTimer = new QTimer(this);
    quitTimer->setSingleShot(true);
    connect(quitTimer, &QTimer::timeout, qApp, QCoreApplication::quit);
    quitTimer->start(1000);
}

void JobRunner::runScheduledJobs(QMap<QString, JobPtr> jobMap)
{
    TSettings settings;
    QDate     now(QDate::currentDate());
    QDate nextDaily  = settings.value("app/next_daily_timestamp", "").toDate();
    QDate nextWeekly = settings.value("app/next_weekly_timestamp", "").toDate();
    QDate nextMonthly =
        settings.value("app/next_monthly_timestamp", "").toDate();
    bool doDaily   = false;
    bool doWeekly  = false;
    bool doMonthly = false;
    if(!nextDaily.isValid() || (nextDaily <= now))
    {
        doDaily = true;
        settings.setValue("app/next_daily_timestamp", now.addDays(1));
    }
    if(!nextWeekly.isValid() || (nextWeekly <= now))
    {
        doWeekly         = true;
        QDate nextSunday = now.addDays(1);
        for(; nextSunday.dayOfWeek() != 7; nextSunday = nextSunday.addDays(1))
            /* Do nothing. */
            ;
        settings.setValue("app/next_weekly_timestamp", nextSunday);
    }
    if(!nextMonthly.isValid() || (nextMonthly <= now))
    {
        doMonthly       = true;
        QDate nextMonth = now.addMonths(1);
        nextMonth.setDate(nextMonth.year(), nextMonth.month(), 1);
        settings.setValue("app/next_monthly_timestamp", nextMonth);
    }
    settings.sync();
    DEBUG << "Daily: " << doDaily;
    DEBUG << "Next daily: "
          << settings.value("app/next_daily_timestamp", "").toDate().toString();
    DEBUG << "Weekly: " << doWeekly;
    DEBUG
        << "Next weekly: "
        << settings.value("app/next_weekly_timestamp", "").toDate().toString();
    DEBUG << "Monthly: " << doWeekly;
    DEBUG
        << "Next monthly: "
        << settings.value("app/next_monthly_timestamp", "").toDate().toString();
    bool nothingToDo = true;
    for(const JobPtr &job : jobMap)
    {
        // Do we need to run any jobs?
        if((doDaily && (job->optionScheduledEnabled() == JobSchedule::Daily))
           || (doWeekly
               && (job->optionScheduledEnabled() == JobSchedule::Weekly))
           || (doMonthly
               && (job->optionScheduledEnabled() == JobSchedule::Monthly)))
        {
            // Before the first job...
            if(nothingToDo)
            {
                // ... we have a job now
                nothingToDo = false;
                // ... check & wait for an internet connection
                if(!waitForOnline())
                    return;
            }
            emit backup(BackupTaskData::createBackupTaskFromJob(job));
        }
    }
    if(nothingToDo)
        qApp->quit();
}

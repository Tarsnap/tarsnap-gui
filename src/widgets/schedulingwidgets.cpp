#include "schedulingwidgets.h"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAbstractButton>
#include <QMessageBox>
#include <QWidget>
WARNINGS_ENABLE

#include "scheduling.h"

SchedulingWidgets::SchedulingWidgets(QWidget *parent)
    : QObject(parent), _parent(parent)
{
}

void SchedulingWidgets::enableJobSchedulingButtonClicked()
{
#if defined(Q_OS_OSX)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(_parent, tr("Job scheduling"),
                              tr("Register Tarsnap GUI with the OS X"
                                 " Launchd service to run daily at 10am?"
                                 "\n\nJobs that have scheduled backup"
                                 " turned on will be backed up according"
                                 " to the Daily, Weekly or Monthly"
                                 " schedule."));
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = launchdEnable();
    if(info.status != SCHEDULE_OK)
    {
        QMessageBox::critical(_parent, tr("Job scheduling"), info.message);
        return;
    }

#elif defined(Q_OS_LINUX) || defined(Q_OS_BSD4)

    QMessageBox::StandardButton confirm =
        QMessageBox::question(_parent, tr("Job scheduling"),
                              tr("Register Tarsnap GUI with cron serivce?"
                                 "\nJobs that have scheduled backup"
                                 " turned on will be backed up according"
                                 " to the Daily, Weekly or Monthly"
                                 " schedule."));
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = cronEnable();
    if(info.status == SCHEDULE_ERROR)
    {
        QMessageBox::critical(_parent, tr("Job scheduling"), info.message);
        return;
    }
    else if(info.status == SCHEDULE_OK)
    {
        QMessageBox::critical(_parent, tr("Job scheduling"),
                              "Unknown error in scheduling code.");
        return;
    }
    QString cronBlock = info.message;

    QMessageBox question(_parent);
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
    for(QAbstractButton *button : question.buttons())
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
        QMessageBox::critical(_parent, tr("Job scheduling"), info.message);
        return;
    }
#endif
}

void SchedulingWidgets::disableJobSchedulingButtonClicked()
{
#if defined(Q_OS_OSX)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(_parent, tr("Job scheduling"),
                              tr("Unregister Tarsnap GUI from the OS X"
                                 " Launchd service? This will disable"
                                 " automatic Job backup scheduling."));
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = launchdDisable();
    if(info.status != SCHEDULE_OK)
    {
        QMessageBox::critical(_parent, tr("Job scheduling"), info.message);
        return;
    }

#elif defined(Q_OS_LINUX) || defined(Q_OS_BSD4)
    QMessageBox::StandardButton confirm =
        QMessageBox::question(_parent, "Confirm action",
                              "Unregister Tarsnap GUI from cron?");
    if(confirm != QMessageBox::Yes)
        return;

    struct scheduleinfo info = cronDisable();
    if(info.status == SCHEDULE_ERROR)
    {
        QMessageBox::critical(_parent, tr("Job scheduling"), info.message);
        return;
    }
    else if(info.status == SCHEDULE_OK)
    {
        QMessageBox::critical(_parent, tr("Job scheduling"),
                              "Unknown error in scheduling code.");
        return;
    }
    QString linesToRemove = info.message;

    QMessageBox question(_parent);
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
    for(QAbstractButton *button : question.buttons())
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
        QMessageBox::critical(_parent, tr("Job scheduling"), info.message);
        return;
    }
#endif
}

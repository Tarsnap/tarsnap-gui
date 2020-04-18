#ifndef JOBRUNNER_H
#define JOBRUNNER_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QMap>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "messages/backuptaskdataptr.h"
#include "messages/jobptr.h"
#include "messages/notification_info.h"

/*!
 * \ingroup background-tasks
 * \brief The JobRunner is a QObject which manages scheduled jobs.
 */
class JobRunner : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    JobRunner();

    //! Checks if any scheduled jobs need to run now; if so, adds them to
    //! the queue.  If there are no scheduled jobs, quit the app immediately.
    void runScheduledJobs(QMap<QString, JobPtr> jobMap);

signals:
    //! A status message should be shown to the user.
    //! \param msg main text to display.
    //! \param detail display this text as a mouse-over tooltip.
    void message(const QString &msg, const QString &detail = "");
    //! A message that should be shown as a desktop notification (if enabled).
    void displayNotification(const QString &message, enum message_type type,
                             const QString &data);
    //! Create a backup
    void backup(BackupTaskDataPtr backupTaskData);

private:
    bool waitForOnline();
    void warnNotOnline();
};

#endif /* !JOBRUNNER_H */

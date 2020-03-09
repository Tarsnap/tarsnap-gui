#ifndef STOPTASKSDIALOG_H
#define STOPTASKSDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QMessageBox>
WARNINGS_ENABLE

/* Forward declarations. */
class QAction;
class QMenu;
class QPushButton;

/*!
 * \ingroup widgets-specialized
 * \brief The StopTasksDialog is a QMessageBox which gives the
 * user the option to stop running or queued tasks.
 */
class StopTasksDialog : public QMessageBox
{
    Q_OBJECT

public:
    //! Constructor.
    explicit StopTasksDialog(QWidget *parent = nullptr);

    //! Prompt user to clarify whether to stop background tasks; if so,
    //! indicate that we can quit the app.  the app.  Also used when quitting
    //! the application while active or background tasks are queued.
    void display(bool backupTaskRunning, int runningTasks, int queuedTasks,
                 bool aboutToQuit);

signals:
    //! Stop / interrupt / terminate / dequeue tasks.
    //! \param interrupt Kill the first task.  \warning MacOS X only.  (?)
    //! \param running Stop all running tasks.
    //! \param queued Remove all tasks from the queue.
    void stopTasks(bool interrupt, bool running, bool queued);
    //! Cancel aboutToQuit.
    void cancelAboutToQuit();
    //! We can actually quit now.
    void quitOk();

private:
    QPushButton *_actionButton;
    QMenu *      _actionMenu;

    QAction *_interruptBackup;
    QAction *_stopRunning;
    QAction *_stopQueued;
    QAction *_stopAll;
    QAction *_proceedBackground;

    bool _aboutToQuit;
};

#endif // !STOPTASKSDIALOG_H

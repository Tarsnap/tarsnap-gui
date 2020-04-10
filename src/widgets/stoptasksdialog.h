#ifndef STOPTASKSDIALOG_H
#define STOPTASKSDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QObject>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class StopTasksDialog;
}
class QWidget;

/*!
 * \ingroup widgets-specialized
 * \brief The StopTasksDialog is a QDialog which gives the
 * user the option to stop running or queued tasks.
 *
 * There are two sets of possible actions.  If we are about to quit,
 *   - stop all
 *   - interrupt backup and clear queue
 *   - proceed in background
 * If we are not about to quit,
 *   - stop all
 *   - interrupt backup but keep queue
 *   - stop running
 *   - clear queue
 */
class StopTasksDialog : public QDialog
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
    friend class TestSmallWidgets;
#endif

public:
    //! Constructor.
    explicit StopTasksDialog(QWidget *parent = nullptr);
    ~StopTasksDialog() override;

    //! Prompt user to clarify whether to stop background tasks; if so,
    //! indicate that we can quit the app.  the app.  Also used when quitting
    //! the application while active or background tasks are queued.
    void display(bool backupTaskRunning, int runningTasks, int queuedTasks,
                 bool aboutToQuit);

public slots:
    //! Update the task numbers and the available actions.
    void updateTasks(bool backupTaskRunning, int runningTasks, int queuedTasks);

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

private slots:
    // The dialog was closed.
    void processResult(int res);

    void interruptBackupClicked();
    void interruptBackupKeepQueueClicked();
    void stopQueuedClicked();
    void stopRunningClicked();
    void stopAllClicked();
    void proceedBackgroundClicked();

private:
    Ui::StopTasksDialog *_ui;

    bool _aboutToQuit;

    void adjust_for_quit(bool quitting);
};

#endif // !STOPTASKSDIALOG_H

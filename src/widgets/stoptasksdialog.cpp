#include "stoptasksdialog.h"

WARNINGS_DISABLE
#include <QAction>
#include <QMenu>
#include <QPushButton>
WARNINGS_ENABLE

StopTasksDialog::StopTasksDialog(QWidget *parent) : QMessageBox(parent)
{
}

void StopTasksDialog::display(bool backupTaskRunning, int runningTasks,
                              int queuedTasks, bool aboutToQuit)
{
    _aboutToQuit = aboutToQuit;

    setText(tr("There are %1 running tasks and %2 queued.")
                .arg(runningTasks)
                .arg(queuedTasks));
    setInformativeText(tr("What do you want to do?"));

    QPushButton actionButton(this);
    actionButton.setText(tr("Choose action"));
    QMenu    actionMenu(&actionButton);
    QAction *interruptBackup = nullptr;
    if(backupTaskRunning)
    {
        if(_aboutToQuit)
            interruptBackup =
                actionMenu.addAction(tr("Interrupt backup and clear queue"));
        else
            interruptBackup = actionMenu.addAction(tr("Interrupt backup"));
        interruptBackup->setCheckable(true);
    }
    QAction *stopRunning = nullptr;
    if(runningTasks && !_aboutToQuit)
    {
        stopRunning = actionMenu.addAction(tr("Stop running"));
        stopRunning->setCheckable(true);
    }
    QAction *stopQueued = nullptr;
    if(queuedTasks && !_aboutToQuit)
    {
        stopQueued = actionMenu.addAction(tr("Cancel queued"));
        stopQueued->setCheckable(true);
    }
    QAction *stopAll = nullptr;
    if(runningTasks || queuedTasks)
    {
        stopAll = actionMenu.addAction(tr("Stop all"));
        stopAll->setCheckable(true);
    }
    QAction *proceedBackground = nullptr;
    if((runningTasks || queuedTasks) && _aboutToQuit)
    {
        proceedBackground = actionMenu.addAction(tr("Proceed in background"));
        proceedBackground->setCheckable(true);
    }
    QPushButton *cancel = addButton(QMessageBox::Cancel);
    setDefaultButton(cancel);
    connect(&actionMenu, &QMenu::triggered, this, &QDialog::accept,
            Qt::QueuedConnection);
    actionButton.setMenu(&actionMenu);
    addButton(&actionButton, QMessageBox::ActionRole);
    int result = exec();

    // If we close the dialog with close() -- e.g., via the QTest
    // framework -- then clickedButton() can be nullptr.
    if((result != QDialog::Accepted) && _aboutToQuit)
    {
        emit cancelAboutToQuit();
        return;
    }

    if(_aboutToQuit)
    {
        emit quitOk();
    }

    if(interruptBackup && interruptBackup->isChecked())
        emit stopTasks(true, false, _aboutToQuit);
    else if(stopQueued && stopQueued->isChecked())
        emit stopTasks(false, false, true);
    else if(stopRunning && stopRunning->isChecked())
        emit stopTasks(false, true, false);
    else if(stopAll && stopAll->isChecked())
        emit stopTasks(false, true, true);
    else if(proceedBackground && proceedBackground->isChecked())
    {
        // Do nothing; it will happen due to code elsewhere
    }
}

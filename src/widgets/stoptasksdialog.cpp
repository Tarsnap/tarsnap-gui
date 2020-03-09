#include "stoptasksdialog.h"

WARNINGS_DISABLE
#include <QAction>
#include <QMenu>
#include <QPushButton>
WARNINGS_ENABLE

StopTasksDialog::StopTasksDialog(QWidget *parent)
    : QMessageBox(parent),
      _actionButton(new QPushButton(this)),
      _actionMenu(new QMenu(_actionButton))
{
}

void StopTasksDialog::display(bool backupTaskRunning, int runningTasks,
                              int queuedTasks, bool aboutToQuit)
{
    _aboutToQuit = aboutToQuit;

    // Overall setup.
    setText(tr("There are %1 running tasks and %2 queued.")
                .arg(runningTasks)
                .arg(queuedTasks));
    setInformativeText(tr("What do you want to do?"));

    _actionButton->setText(tr("Choose action"));

    // interrupt
    QAction *interruptBackup = nullptr;
    if(backupTaskRunning)
    {
        if(_aboutToQuit)
            interruptBackup =
                _actionMenu->addAction(tr("Interrupt backup and clear queue"));
        else
            interruptBackup = _actionMenu->addAction(tr("Interrupt backup"));
        interruptBackup->setCheckable(true);
    }
    // stopRunning
    QAction *stopRunning = nullptr;
    if(runningTasks && !_aboutToQuit)
    {
        stopRunning = _actionMenu->addAction(tr("Stop running"));
        stopRunning->setCheckable(true);
    }
    // stopQueued
    QAction *stopQueued = nullptr;
    if(queuedTasks && !_aboutToQuit)
    {
        stopQueued = _actionMenu->addAction(tr("Cancel queued"));
        stopQueued->setCheckable(true);
    }
    // stopAll
    QAction *stopAll = nullptr;
    if(runningTasks || queuedTasks)
    {
        stopAll = _actionMenu->addAction(tr("Stop all"));
        stopAll->setCheckable(true);
    }
    // proceedBackground
    QAction *proceedBackground = nullptr;
    if((runningTasks || queuedTasks) && _aboutToQuit)
    {
        proceedBackground = _actionMenu->addAction(tr("Proceed in background"));
        proceedBackground->setCheckable(true);
    }
    QPushButton *cancel = addButton(QMessageBox::Cancel);
    setDefaultButton(cancel);
    connect(_actionMenu, &QMenu::triggered, this, &QDialog::accept,
            Qt::QueuedConnection);
    _actionButton->setMenu(_actionMenu);
    addButton(_actionButton, QMessageBox::ActionRole);

    // Launch dialog.
    int result = exec();

    // If we close the dialog with close() -- e.g., via the QTest
    // framework -- then clickedButton() can be nullptr.
    if((result != QDialog::Accepted) && _aboutToQuit)
    {
        emit cancelAboutToQuit();
        return;
    }

    // Signal that we can quit now.
    if(_aboutToQuit)
    {
        emit quitOk();
    }

    // Stop tasks.
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

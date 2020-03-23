#include "stoptasksdialog.h"

WARNINGS_DISABLE
#include <QAction>
#include <QMenu>
#include <QPushButton>
WARNINGS_ENABLE

StopTasksDialog::StopTasksDialog(QWidget *parent) : QMessageBox(parent)
{
    // Set up text.
    setInformativeText(tr("What do you want to do?"));

    // Set up menu.
    _actionButton = new QPushButton(this);
    _actionMenu   = new QMenu(_actionButton);
    _actionButton->setText(tr("Choose action"));
    _actionButton->setMenu(_actionMenu);
    addButton(_actionButton, QMessageBox::ActionRole);

    // Set up menu actions.
    _interruptBackup =
        _actionMenu->addAction(tr("Interrupt backup and clear queue"));
    _interruptBackup->setCheckable(true);
    connect(_interruptBackup, &QAction::triggered, this,
            &StopTasksDialog::interruptBackupClicked);
    _interruptBackupKeepQueue = _actionMenu->addAction(tr("Interrupt backup"));
    _interruptBackupKeepQueue->setCheckable(true);
    connect(_interruptBackupKeepQueue, &QAction::triggered, this,
            &StopTasksDialog::interruptBackupKeepQueueClicked);
    _stopRunning = _actionMenu->addAction(tr("Stop running"));
    _stopRunning->setCheckable(true);
    connect(_stopRunning, &QAction::triggered, this,
            &StopTasksDialog::stopRunningClicked);
    _stopQueued = _actionMenu->addAction(tr("Cancel queued"));
    _stopQueued->setCheckable(true);
    connect(_stopQueued, &QAction::triggered, this,
            &StopTasksDialog::stopQueuedClicked);
    _stopAll = _actionMenu->addAction(tr("Stop all"));
    _stopAll->setCheckable(true);
    connect(_stopAll, &QAction::triggered, this,
            &StopTasksDialog::stopAllClicked);
    _proceedBackground = _actionMenu->addAction(tr("Proceed in background"));
    _proceedBackground->setCheckable(true);
    connect(_proceedBackground, &QAction::triggered, this,
            &StopTasksDialog::proceedBackgroundClicked);

    // Set up buttons.
    _cancelButton = addButton(QMessageBox::Cancel);
    setDefaultButton(_cancelButton);

    // Set up processing the result.
    connect(this, &QMessageBox::finished, this,
            &StopTasksDialog::processResult);
}

void StopTasksDialog::display(bool backupTaskRunning, int runningTasks,
                              int queuedTasks, bool aboutToQuit)
{
    _aboutToQuit = aboutToQuit;

    // Sanity check.
    Q_ASSERT((runningTasks > 0) || (queuedTasks > 0));

    // Overall setup.
    setText(tr("There are %1 running tasks and %2 queued.")
                .arg(runningTasks)
                .arg(queuedTasks));

    // interrupt
    _interruptBackup->setVisible(backupTaskRunning && _aboutToQuit);
    _interruptBackupKeepQueue->setVisible(backupTaskRunning && !_aboutToQuit);
    // stopRunning
    _stopRunning->setVisible(runningTasks && !_aboutToQuit);
    // stopQueued
    _stopQueued->setVisible(queuedTasks && !_aboutToQuit);
    // stopAll
    _stopAll->setVisible(runningTasks || queuedTasks);
    // proceedBackground
    _proceedBackground->setVisible((runningTasks || queuedTasks)
                                   && _aboutToQuit);

    // Launch dialog.
    open();
}

void StopTasksDialog::interruptBackupClicked()
{
    emit stopTasks(true, false, true);
    accept();
}

void StopTasksDialog::interruptBackupKeepQueueClicked()
{
    emit stopTasks(true, false, false);
    accept();
}

void StopTasksDialog::stopQueuedClicked()
{
    emit stopTasks(false, false, true);
    accept();
}

void StopTasksDialog::stopRunningClicked()
{
    emit stopTasks(false, true, false);
    accept();
}

void StopTasksDialog::stopAllClicked()
{
    emit stopTasks(false, true, true);
    accept();
}

void StopTasksDialog::proceedBackgroundClicked()
{
    // Do nothing special; it will happen due to code elsewhere.

    accept();
}

void StopTasksDialog::processResult(int res)
{
    // Keep 'res' name to avoid shadowing QDialog::result().

    // Bail (if applicable).
    // QMessageBox::finished() is not documented as diverging from
    // QDialog::finished().  However, it does differ, similar to
    // QMessageBox::exec() -- it can return a QMessageBox::StandardButton
    // value.  That said, if the QMessageBox is closed with an
    // ->accept(), ->reject(), or ->close(), it will still provide a
    // QDialog::Accepted or QDialog::Rejected.
    if(res != QDialog::Accepted)
    {
        if(_aboutToQuit)
        {
            _aboutToQuit = false;
            emit cancelAboutToQuit();
        }
        return;
    }

    // Signal that we can quit now.
    if(_aboutToQuit)
    {
        emit quitOk();
    }
}

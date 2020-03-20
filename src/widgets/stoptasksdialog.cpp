#include "stoptasksdialog.h"

WARNINGS_DISABLE
#include <QAction>
#include <QMenu>
#include <QPushButton>
WARNINGS_ENABLE

StopTasksDialog::StopTasksDialog(QWidget *parent)
    : QMessageBox(parent),
      _actionButton(new QPushButton(this)),
      _actionMenu(new QMenu(_actionButton)),
      _cancelButton(nullptr)
{
    // Set up text.
    setInformativeText(tr("What do you want to do?"));
    _actionButton->setText(tr("Choose action"));

    // Set up buttons.
    connect(_actionMenu, &QMenu::triggered, this, &QDialog::accept,
            Qt::QueuedConnection);
    _actionButton->setMenu(_actionMenu);
    addButton(_actionButton, QMessageBox::ActionRole);

    // Set up processing the result.
    connect(this, &QMessageBox::finished, this,
            &StopTasksDialog::processResult);
}

void StopTasksDialog::display(bool backupTaskRunning, int runningTasks,
                              int queuedTasks, bool aboutToQuit)
{
    _aboutToQuit = aboutToQuit;

    // Overall setup.
    setText(tr("There are %1 running tasks and %2 queued.")
                .arg(runningTasks)
                .arg(queuedTasks));
    _actionMenu->clear();

    // Ensure that we have a new cancel button, to avoid
    // clickedButton() carrying over from the previous display().
    QPushButton *cancel_orig = _cancelButton;
    if(_cancelButton != nullptr)
        removeButton(_cancelButton);
    _cancelButton = addButton(QMessageBox::Cancel);
    setDefaultButton(_cancelButton);
    if(cancel_orig != nullptr)
        delete cancel_orig;

    // interrupt
    _interruptBackup = nullptr;
    if(backupTaskRunning)
    {
        if(_aboutToQuit)
            _interruptBackup =
                _actionMenu->addAction(tr("Interrupt backup and clear queue"));
        else
            _interruptBackup = _actionMenu->addAction(tr("Interrupt backup"));
        _interruptBackup->setCheckable(true);
    }
    // stopRunning
    _stopRunning = nullptr;
    if(runningTasks && !_aboutToQuit)
    {
        _stopRunning = _actionMenu->addAction(tr("Stop running"));
        _stopRunning->setCheckable(true);
    }
    // stopQueued
    _stopQueued = nullptr;
    if(queuedTasks && !_aboutToQuit)
    {
        _stopQueued = _actionMenu->addAction(tr("Cancel queued"));
        _stopQueued->setCheckable(true);
    }
    // stopAll
    _stopAll = nullptr;
    if(runningTasks || queuedTasks)
    {
        _stopAll = _actionMenu->addAction(tr("Stop all"));
        _stopAll->setCheckable(true);
    }
    // proceedBackground
    _proceedBackground = nullptr;
    if((runningTasks || queuedTasks) && _aboutToQuit)
    {
        _proceedBackground =
            _actionMenu->addAction(tr("Proceed in background"));
        _proceedBackground->setCheckable(true);
    }

    // Launch dialog.
    open();
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
    if((res != QDialog::Accepted) || (clickedButton() == _cancelButton))
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

    // Stop tasks.
    if(_interruptBackup && _interruptBackup->isChecked())
        emit stopTasks(true, false, _aboutToQuit);
    else if(_stopQueued && _stopQueued->isChecked())
        emit stopTasks(false, false, true);
    else if(_stopRunning && _stopRunning->isChecked())
        emit stopTasks(false, true, false);
    else if(_stopAll && _stopAll->isChecked())
        emit stopTasks(false, true, true);
    else if(_proceedBackground && _proceedBackground->isChecked())
    {
        // Do nothing; it will happen due to code elsewhere
    }
}

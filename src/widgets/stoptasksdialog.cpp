#include "stoptasksdialog.h"

WARNINGS_DISABLE
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include "ui_stoptasksdialog.h"
WARNINGS_ENABLE

/* Forward declaration(s). */
class QWidget;

StopTasksDialog::StopTasksDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::StopTasksDialog), _aboutToQuit(false)
{
    // Ui initialization
    _ui->setupUi(this);

    // Set up buttons.
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this,
            &QDialog::reject);
    connect(_ui->interruptButton, &QPushButton::clicked, this,
            &StopTasksDialog::interruptBackupClicked);
    connect(_ui->interruptKeepQueueButton, &QPushButton::clicked, this,
            &StopTasksDialog::interruptBackupKeepQueueClicked);
    connect(_ui->stopRunningButton, &QPushButton::clicked, this,
            &StopTasksDialog::stopRunningClicked);
    connect(_ui->cancelQueuedButton, &QPushButton::clicked, this,
            &StopTasksDialog::stopQueuedClicked);
    connect(_ui->stopAllButton, &QPushButton::clicked, this,
            &StopTasksDialog::stopAllClicked);
    connect(_ui->proceedBackgroundButton, &QPushButton::clicked, this,
            &StopTasksDialog::proceedBackgroundClicked);

    // Make sure focus is on the Cancel button.
    _ui->buttonBox->button(QDialogButtonBox::Cancel)->setFocus();

    // Set up processing the result.
    connect(this, &QMessageBox::finished, this,
            &StopTasksDialog::processResult);
}

StopTasksDialog::~StopTasksDialog()
{
    delete _ui;
}

void StopTasksDialog::adjust_for_quit(bool quitting)
{
    _ui->interruptButton->setVisible(quitting);
    _ui->proceedBackgroundButton->setVisible(quitting);

    _ui->interruptKeepQueueButton->setVisible(!quitting);
    _ui->stopRunningButton->setVisible(!quitting);
    _ui->cancelQueuedButton->setVisible(!quitting);

    // Alter the "severity" gradient.
    if(quitting)
    {
        _ui->gradientLabel->setStyleSheet(
            "background: qlineargradient(spread:pad,"
            " x1: 0, y1: 0, x2: 0, y2: 1,"
            " stop: 0 rgba(255, 0, 0, 255),"
            " stop: 0.67 rgba(255, 255, 0, 255),"
            " stop: 1 rgba(0, 255, 0, 255))");
    }
    else
    {
        _ui->gradientLabel->setStyleSheet(
            "background: qlineargradient(spread:pad,"
            " x1: 0, y1: 0, x2: 0, y2: 1,"
            " stop: 0 rgba(255, 0, 0, 255),"
            " stop: 1 rgba(255, 255, 0, 255))");
    }

    // Reset height
    adjustSize();
}

void StopTasksDialog::display(bool backupTaskRunning, int runningTasks,
                              int queuedTasks, bool aboutToQuit)
{
    _aboutToQuit = aboutToQuit;
    adjust_for_quit(_aboutToQuit);

    // Sanity check.
    Q_ASSERT((runningTasks > 0) || (queuedTasks > 0));
    Q_ASSERT(!backupTaskRunning || (runningTasks > 0));

    // Update enabled buttons.
    updateTasks(backupTaskRunning, runningTasks, queuedTasks);

    // Launch dialog.
    open();
}

void StopTasksDialog::updateTasks(bool backupTaskRunning, int runningTasks,
                                  int queuedTasks)
{
    // Shut down the dialog (if necesssary).
    if((runningTasks == 0) && (queuedTasks == 0))
        accept();

    // Overall setup.
    _ui->numTasksLabel->setText(tr("There are %1 running tasks and %2 queued.")
                                    .arg(runningTasks)
                                    .arg(queuedTasks));

    // interrupt
    _ui->interruptButton->setEnabled(backupTaskRunning && _aboutToQuit);
    _ui->interruptKeepQueueButton->setEnabled(backupTaskRunning
                                              && !_aboutToQuit);
    // stopRunning
    _ui->stopRunningButton->setEnabled(runningTasks && !_aboutToQuit);
    // stopQueued
    _ui->cancelQueuedButton->setEnabled(queuedTasks && !_aboutToQuit);
    // stopAll
    _ui->stopAllButton->setEnabled(runningTasks || queuedTasks);
    // proceedBackground
    _ui->proceedBackgroundButton->setEnabled((runningTasks || queuedTasks)
                                             && _aboutToQuit);
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

#include "confirmationdialog.h"

ConfirmationDialog::ConfirmationDialog(QWidget *parent)
    : QObject(parent), _inputDialog(parent), _countdownBox(parent)
{
    // Set up text confirmation dialog
    _inputDialog.setInputMode(QInputDialog::TextInput);
    connect(&_inputDialog, &QInputDialog::textValueChanged, this,
            &ConfirmationDialog::validateConfirmationText);
    connect(&_inputDialog, &QInputDialog::finished, this,
            &ConfirmationDialog::finishedConfirmationBox);

    // Set up countdown box
    _countdownBox.setIcon(QMessageBox::Critical);
    _countdownBox.setStandardButtons(QMessageBox::Cancel);
    connect(&_timer, &QTimer::timeout, this, &ConfirmationDialog::timerFired);
}

void ConfirmationDialog::validateConfirmationText(const QString &text)
{
    if(text == _confirmationText)
        _inputDialog.setOkButtonText(_confirmedButtonText);
    else
        _inputDialog.setOkButtonText(tr("Not confirmed"));
}

void ConfirmationDialog::finishedConfirmationBox(int result)
{
    // Bail if it's a cancel or if the text doesn't match.
    if((result != QDialog::Accepted)
       || (_confirmationText != _inputDialog.textValue()))
    {
        emit cancelled();
        return;
    }

    // Launch the countdown.
    _countdownBox.setWindowTitle(_countdownTitle);
    _countdownBox.setText(_countdownText.arg(_countdownSeconds));
    _timer.start(1000);
    if(QMessageBox::Cancel == _countdownBox.exec())
    {
        _timer.stop();
        emit cancelled();
    }
}

void ConfirmationDialog::timerFired()
{
    if(_countdownSeconds <= 1)
    {
        _timer.stop();
        _countdownBox.accept();
        emit confirmed();
    }
    else
    {
        --_countdownSeconds;
        _countdownBox.setText(_countdownText.arg(_countdownSeconds));
    }
}

// Keep "title" in here so that it can be translated.
void ConfirmationDialog::start(const QString &startTitle,
                               const QString &startText,
                               const QString &confirmationText,
                               int            countdownSeconds,
                               const QString &countdownTitle,
                               const QString &countdownText,
                               const QString &confirmedButtonText)
{
    _inputDialog.setWindowTitle(startTitle);
    _inputDialog.setLabelText(startText);
    _inputDialog.setOkButtonText(tr("Not confirmed"));
    _confirmationText    = confirmationText;
    _confirmedButtonText = confirmedButtonText;
    _countdownSeconds    = countdownSeconds;
    _countdownTitle      = countdownTitle;
    _countdownText       = countdownText;

    // Clear any previous text.
    _inputDialog.setTextValue("");

    // Launch dialog.
    _inputDialog.open();
}

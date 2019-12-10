#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
WARNINGS_ENABLE

/*!
 * \ingroup widgets-general
 * \brief The ConfirmationDialog is a QWidget which requires the user to
 * type a confirmation message, then has an additional countdown timer (to
 * allow more cancellation opportunities) before the action is taken.
 */
class ConfirmationDialog : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSettingsWidget;
#endif

public:
    //! Constructor.
    explicit ConfirmationDialog(QWidget *parent = nullptr);

    //! Display a confirmation dialog which requires the user to type some
    //! text, and then must wait for a countdown timer.
    //! \param startTitle title of the typing confirmation window
    //! \param startText text of the typing confirmation window
    //! \param confirmationText what the user must type
    //! \param countdownSeconds how many seconds the user must wait
    //! \param countdownTitle title of the countdown window
    //! \param countdownText text of the countdown window
    //! \param confirmedButtonText what to display on the "Ok" button
    //!                            after text confirmation
    void start(const QString &startTitle, const QString &startText,
               const QString &confirmationText, int countdownSeconds,
               const QString &countdownTitle, const QString &countdownText,
               const QString &confirmedButtonText);

signals:
    //! User has confirmed the action.
    void confirmed();
    //! User has not confirmed, or cancelled, the action.
    void cancelled();

private slots:
    void validateConfirmationText(const QString &text);
    void timerFired();

private:
    QInputDialog _inputDialog;
    QString      _confirmationText;

    QMessageBox _countdownBox;
    QTimer      _timer;
    int         _countdownSeconds;
    QString     _countdownText;

    QString _confirmedButtonText;
};

#endif // CONFIRMDIALOG_H

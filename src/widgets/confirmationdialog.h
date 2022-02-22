#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QInputDialog;
class QMessageBox;
class QTimer;
class QWidget;

/*!
 * \ingroup widgets-general
 * \brief The ConfirmationDialog is a QObject which requires the user to
 * type a confirmation message, then has an additional countdown timer (to
 * allow more cancellation opportunities) before the action is taken.
 *
 * This widget does not support dynamic translation, as it is a modal dialog
 * so it does not allow the user to change the settings while visible.
 */
class ConfirmationDialog : public QObject
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSettingsWidget;
    friend class TestSmallWidgets;
    friend class TestTranslations;
#endif

public:
    //! Constructor.
    explicit ConfirmationDialog(QWidget *parent = nullptr);
    ~ConfirmationDialog() override;

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
    void finishedConfirmationBox(int result);
    void timerFired();
    void finishedCountdownBox(int result);

private:
    // Confirmation text
    QInputDialog *_inputDialog;
    QString       _confirmationText;
    QString       _confirmedButtonText;

    // Countdown box
    QMessageBox *_countdownBox;
    QTimer      *_timer;
    int          _countdownSeconds;
    QString      _countdownTitle;
    QString      _countdownText;
};

#endif // !CONFIRMATIONDIALOG_H

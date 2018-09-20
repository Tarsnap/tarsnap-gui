#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>

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

    void start(QString startTitle, QString startText, QString confirmationText,
               int countdownSeconds, QString countdownTitle,
               QString countdownText);

signals:
    //! User has confirmed the action.
    void confirmed();
    //! User has not confirmed, or cancelled, the action.
    void cancelled();

private slots:
    void timerFired();

private:
    QInputDialog _inputDialog;
    QString      _confirmationText;

    QMessageBox _countdownBox;
    QTimer      _timer;
    int         _countdownSeconds;
    QString     _countdownText;
};

#endif // CONFIRMDIALOG_H

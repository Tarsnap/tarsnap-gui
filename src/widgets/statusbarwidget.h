#ifndef STATUSBARWIDGET_H
#define STATUSBARWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class StatusBarWidget;
}
class QEvent;

/*!
 * \ingroup widgets-main
 * \brief The StatusBarWidget is a QWidget which displays status information.
 */
class StatusBarWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit StatusBarWidget(QWidget *parent = nullptr);
    ~StatusBarWidget();

    //! Indicate whether the backend is busy or not.
    void showBusy(bool busy);

public slots:
    //! Set the statusbar message.
    //! \param message display this text
    //! \param detail display this text as a mouse-over tooltip.
    void updateStatusMessage(const QString &message, const QString &detail);

    //! Update the simulation icon.
    void updateSimulationIcon(int state);

signals:
    //! We want to stop some tasks.
    void stopTasksRequested();
    //! We want to toggle the journal log visibility.
    void journalToggleRequested();

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private:
    Ui::StatusBarWidget *_ui;

    void updateUi();
};

#endif /* !STATUSBARWIDGET_H */

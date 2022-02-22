#ifndef APP_GUI_H
#define APP_GUI_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QApplication>
#include <QObject>
WARNINGS_ENABLE

extern "C" {
#include "optparse.h"
}

/* Forward declaration(s). */
class Journal;
class MainWindow;
class Notification;
class TaskManager;

/*!
 * \ingroup misc
 * \brief The AppGui is a QApplication which launches the MainWindow,
 * and manages background tasks.
 */
class AppGui : public QApplication
{
    Q_OBJECT

public:
    //! Constructor.
    AppGui(int &argc, char **argv, struct optparse *opt);
    ~AppGui() override;

    //! Handle the initialization
    bool handle_init(const QList<struct init_info> &steps);

    //! Prepare to pass control to the QEventLoop.
    //! \return True if the calling function should call app.exec().
    bool prepEventLoop();

private slots:
    //! Restart so that we can run the SetupWizard.
    void reinit();
    //! Creates a MainWindow.
    void showMainWindow();
    //! Quit the app after any remaining events have finished.
    void quitAfterEventsFinish();

private:
    MainWindow   *_mainWindow;
    Journal      *_journal;
    Notification *_notification;
    TaskManager  *_taskManager;
    bool          _jobsOption;
    QString       _configDir;

    bool handle_step(const struct init_info &info);
};

#endif // APP_GUI_H

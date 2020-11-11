#ifndef APP_SETUP_H
#define APP_SETUP_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QApplication>
#include <QList>
#include <QObject>
WARNINGS_ENABLE

/* Forward declaration(s). */
class SetupWizard;
class TaskManager;

/*!
 * \ingroup misc
 * \brief The AppSetup is a QApplication which handles the setup wizard.
 */
class AppSetup : public QApplication
{
    Q_OBJECT

public:
    //! Constructor.
    AppSetup(int &argc, char **argv);
    ~AppSetup() override;

    //! Handle the initialization
    bool handle_init(const QList<struct init_info> &steps);

    //! Prepare to pass control to the QEventLoop.
    //! \return True if the calling function should call app.exec().
    bool prepEventLoop();

#ifdef QT_TESTLIB_LIB
    //! Don't actually run the next task.
    void fakeNextTask();
    //! Block until there's no tasks.
    void waitUntilIdle();
    //! Get the wizard.
    SetupWizard *get_wizard() { return _wizard; }
#endif

#ifdef QT_TESTLIB_LIB
signals:
    //! The QEventLoop has started.
    void eventLoopStarted(AppSetup *setup);
#endif

private slots:
    //! The wizard is finished.
    //! \param result QDialog::Accepted or QDialog::Rejected.
    void finished(int result);

#ifdef QT_TESTLIB_LIB
    //! Event loop has started
    void onEventLoopStarted();
#endif

private:
    SetupWizard *_wizard;
    TaskManager *_taskManager;

    bool handle_step(const struct init_info &info);
};

#endif // APP_SETUP_H

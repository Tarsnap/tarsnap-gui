#ifndef APP_CMDLINE_H
#define APP_CMDLINE_H

#include <QCoreApplication>
#include <QObject>

extern "C" {
#include "optparse.h"
}

/*!
 * \ingroup misc
 * \brief The AppCmdline is a QCoreApplication which manages background tasks.
 */
class AppCmdline : public QCoreApplication
{
    Q_OBJECT

public:
    //! Constructor.
    AppCmdline(int &argc, char **argv, struct optparse *opt);
    ~AppCmdline();

    //! Initializes the Settings, Translator, PersistentStore, and launches
    //! the Setup wizard (if necessary).
    bool initializeCore();

    //! Prepare to pass control to the QEventLoop.
    //! \return True if the calling function should call app.exec().
    bool prepMainLoop();

private:
    QString _configDir;
    bool    _checkOption;
};

#endif // APP_CMDLINE_H

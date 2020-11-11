#ifndef APP_CMDLINE_H
#define APP_CMDLINE_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QList>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
struct optparse;

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
    ~AppCmdline() override;

    //! Handle the initialization
    bool handle_init(const QList<struct init_info> &steps);

    //! Prepare to pass control to the QEventLoop.
    //! \return True if the calling function should call app.exec().
    bool prepEventLoop();

private:
    QString _configDir;
    bool    _checkOption;

    bool handle_step(const struct init_info &info);
};

#endif // APP_CMDLINE_H

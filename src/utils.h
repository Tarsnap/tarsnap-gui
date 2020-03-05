#ifndef UTILS_H
#define UTILS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDir>
#include <QRunnable>
#include <QStringList>
WARNINGS_ENABLE

#define DEFAULT_TRAVERSE_MOUNT true
#define DEFAULT_FOLLOW_SYMLINKS false
#define DEFAULT_DRY_RUN false
#define DEFAULT_SKIP_NODUMP false
#define DEFAULT_SKIP_FILES_SIZE 0

#define DEFAULT_SKIP_SYSTEM_ENABLED false
#if defined Q_OS_OSX
#define DEFAULT_SKIP_SYSTEM_FILES                                              \
    ".DS_Store:.localized:.fseventsd:.Spotlight-V100:._.Trashes:.Trashes"
#elif defined Q_OS_WIN
#define DEFAULT_SKIP_SYSTEM_FILES                                              \
    "$RECYCLE.BIN:System Volume Information:Thumbs.db"
#elif defined Q_OS_LINUX
#define DEFAULT_SKIP_SYSTEM_FILES ".lost+found"
#else
#define DEFAULT_SKIP_SYSTEM_FILES ""
#endif

#define DEFAULT_AGGRESSIVE_NETWORKING false

#define QUEUED Qt::QueuedConnection

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define APPDATA QStandardPaths::AppLocalDataLocation
#else
#define APPDATA QStandardPaths::DataLocation
#endif

#define CRON_LINE "*/30 * * * * /usr/bin/env %1 %2 %3 %4 --jobs"
#define CRON_MARKER_BEGIN "#BEGIN Tarsnap GUI - DO NOT EDIT THIS BLOCK MANUALLY"
#define CRON_MARKER_END "#END Tarsnap GUI - DO NOT EDIT THIS BLOCK MANUALLY"
#define CRON_MARKER_HELP                                                       \
    "# For help, see: https://github.com/Tarsnap/tarsnap-gui/wiki/Scheduling"

//! A directory name or an error message.
struct DirMessage
{
    //! Directory which contains the binary, or empty.
    QString dirname;
    //! Error message (if applicable).
    QString errorMessage;
};

namespace Utils
{

/*!
 * \ingroup background-tasks
 * \brief The GetDirInfoTask reads the filesize and count of a directory
 * and its subdirectories.
 */
class GetDirInfoTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    //! Constructor.
    explicit GetDirInfoTask(QDir dir);
    //! Execute the task.
    void run();

signals:
    //! The directory's size and number of files.
    void result(quint64 size, quint64 count);

private:
    QDir _dir;

    quint64 getDirSize(QDir dir);
    quint64 getDirCount(QDir dir);
};

// Convert an int64 size in bytes to a human readable string using either
// SI(1000) or IEC(1024) units (default is SI) depending on the app setting
QString humanBytes(quint64 bytes, int fieldWidth = 0);

// if path.isEmpty it will search in $PATH
// if keygenToo it will search for tarsnap-keygen too
// returns the directory where tarsnap resides if found, otherwise empty string
struct DirMessage findTarsnapClientInPath(QString path, bool keygenToo = false);

// Search for valid tarsnap keys in the supplied path
QFileInfoList findKeysInPath(QString path);

//! Return the reason why dirname is not valid, or empty string for success.
const QString validate_writeable_dir(const QString &dirname);

//! Return the reason why filename is not valid, or empty string for success.
const QString validate_readable_file(const QString &filename);

// Verifies if the current CLI utils version is at least minVersion
bool tarsnapVersionMinimum(const QString &minVersion);

// Displays a "copy&paste-able" command line.
QString quoteCommandLine(QStringList args);
} // namespace Utils

#endif // UTILS_H

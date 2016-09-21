#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QRunnable>

#define CMD_TARSNAP         "tarsnap"
#define CMD_TARSNAPKEYGEN   "tarsnap-keygen"
#define CMD_TARSNAPKEYMGMT  "tarsnap-keymgmt"

#define DEFAULT_PRESERVE_PATHNAMES true
#define DEFAULT_TRAVERSE_MOUNT true
#define DEFAULT_FOLLOW_SYMLINKS false
#define DEFAULT_DRY_RUN false
#define DEFAULT_SKIP_NODUMP false
#define DEFAULT_SKIP_FILES_SIZE 0

#define DEFAULT_SKIP_SYSTEM_ENABLED false
#if defined Q_OS_OSX
#define DEFAULT_SKIP_SYSTEM_FILES ".DS_Store:.localized:.fseventsd:.Spotlight-V100:._.Trashes:.Trashes"
#elif defined Q_OS_WIN
#define DEFAULT_SKIP_SYSTEM_FILES "$RECYCLE.BIN:System Volume Information:Thumbs.db"
#elif defined Q_OS_LINUX
#define DEFAULT_SKIP_SYSTEM_FILES ".lost+found"
#else
#define DEFAULT_SKIP_SYSTEM_FILES ""
#endif

#define DEFAULT_AGGRESSIVE_NETWORKING false
#define DEFAULT_NO_DEFAULT_CONFIG true

#define DEFAULT_DOWNLOADS \
    QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)

const QStringList DEFAULT_JOBS {
    "Desktop",
    "Documents",
    "Pictures",
    "Movies",
    "Videos",
    "Music",
    "Work"
};

#define QUEUED Qt::QueuedConnection

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define APPDATA QStandardPaths::AppLocalDataLocation
#else
#define APPDATA QStandardPaths::DataLocation
#endif

namespace Utils
{

class GetDirInfoTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit GetDirInfoTask(QDir dir);
    void run();

signals:
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
QString findTarsnapClientInPath(QString path, bool keygenToo = false);

// Search for valid tarsnap keys in the supplied path
QFileInfoList findKeysInPath(QString path);

// Returns canonical path if verified, otherwise empty string
QString validateTarsnapCache(QString path);

// Verifies if the current CLI utils version is at least minVersion
bool tarsnapVersionMinimum(const QString &minVersion);

} // namespace Utils

#endif // UTILS_H

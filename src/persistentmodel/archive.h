#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "persistentobject.h"

#include <QDateTime>
#include <QObject>
#include <QRunnable>
#include <QSharedPointer>
#include <QStringList>

#define ARCHIVE_TIMESTAMP_FORMAT QLatin1String("_yyyy-MM-dd_HH-mm-ss")

struct ArchiveRestoreOptions
{
    bool        optionRestore    = false;
    bool        optionRestoreDir = false;
    bool        optionTarArchive = false;
    bool        overwriteFiles   = false;
    bool        keepNewerFiles   = true;
    bool        preservePerms    = false;
    QString     path;
    QStringList files;
};

class Archive;
typedef QSharedPointer<Archive> ArchivePtr;

Q_DECLARE_METATYPE(ArchivePtr)

struct File
{
    QString name;
    QString modified;
    quint64 size;
    QString user;
    QString group;
    QString mode;
    quint64 links;
};

/*!
 * \ingroup background-tasks
 * \brief The ParseArchiveListingTask extracts the list of files
 * from an archive.
 */
class ParseArchiveListingTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    //! Constructor.
    //! \param listing the output of <tt>tarsnap -tv</tt>.
    explicit ParseArchiveListingTask(const QString &listing) : _listing(listing)
    {
    }
    //! Run this task in the background; will emit the \ref result
    //! signal when finished.
    void run();

signals:
    //! The list of files.
    void result(QVector<File> files);

private:
    QString _listing;
};

/*!
 * \ingroup persistent
 * \brief The Archive stores metadata about a user's archive.
 */
class Archive : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    //! Constructor.
    explicit Archive(QObject *parent = nullptr);
    ~Archive();

    //! \name Getter/setter methods
    //! @{
    QString name() const;
    void setName(const QString &value);
    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &value);
    bool truncated() const;
    void setTruncated(bool truncated);
    QString truncatedInfo() const;
    void setTruncatedInfo(const QString &truncatedInfo);
    quint64 sizeTotal() const;
    void setSizeTotal(const quint64 &value);
    quint64 sizeCompressed() const;
    void setSizeCompressed(const quint64 &value);
    quint64 sizeUniqueTotal() const;
    void setSizeUniqueTotal(const quint64 &value);
    quint64 sizeUniqueCompressed() const;
    void setSizeUniqueCompressed(const quint64 &value);
    QString command() const;
    void setCommand(const QString &value);
    QString contents() const;
    void setContents(const QString &value);
    QString jobRef() const;
    void setJobRef(const QString &jobRef);
    //! @}

    //! Starts loading the file list.  When finished, it emits a \ref fileList
    //! signal.
    void getFileList();
    //! Returns whether the tarsnap command included "-P" (preserve pathnames).
    bool hasPreservePaths();
    //! Returns whether this Archive has been scheduled for deletion.
    bool deleteScheduled() const;
    //! Sets whether this Archive is scheduled for deletion. Emits changed().
    void setDeleteScheduled(bool deleteScheduled);

    // From PersistentObject
    //! Saves this object to the PersistentStore; creating or
    //! updating as appropriate.
    void save();
    //! Loads this object from the PersistentStore.  The object's
    //! \c _name must already be set.
    void load();
    //! Deletes this object from the PersistentStore.  The object's
    //! \c _name must already be set.
    void purge();
    //! Returns whether an object with this key exists in the PersistentStore.
    bool doesKeyExist(QString key);

public slots:
    //! Returns statistics about this archive.
    QString archiveStats();

signals:
    //! This item was saved.
    void changed();
    //! This item was deleted.
    void purged();
    //! The file list has been updated.
    void fileList(QVector<File> files);

private:
    QString    _name;
    QDateTime  _timestamp;
    bool       _truncated;
    QString    _truncatedInfo;
    quint64    _sizeTotal;
    quint64    _sizeCompressed;
    quint64    _sizeUniqueTotal;
    quint64    _sizeUniqueCompressed;
    QString    _command;
    QByteArray _contents;
    QString    _jobRef;

    // Properties not saved to the PersistentStore
    bool _deleteScheduled;
};

#endif // ARCHIVE_H

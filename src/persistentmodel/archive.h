#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QByteArray>
#include <QDateTime>
#include <QMetaType>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "messages/archiveptr.h"

#include "persistentmodel/persistentobject.h"

#define ARCHIVE_TIMESTAMP_FORMAT QLatin1String("_yyyy-MM-dd_HH-mm-ss")

Q_DECLARE_METATYPE(ArchivePtr)

/*!
 * \ingroup persistent
 * \brief The Archive stores metadata about a user's archive.
 */
class Archive : public PersistentObject
{
    Q_OBJECT

public:
    //! Constructor.
    explicit Archive(QObject *parent = nullptr);

    //! Getter/setter methods
    //! @{
    QString   name() const;
    void      setName(const QString &value);
    QDateTime timestamp() const;
    void      setTimestamp(const QDateTime &value);
    bool      truncated() const;
    void      setTruncated(bool truncated);
    QString   truncatedInfo() const;
    void      setTruncatedInfo(const QString &truncatedInfo);
    quint64   sizeTotal() const;
    void      setSizeTotal(const quint64 &value);
    quint64   sizeCompressed() const;
    void      setSizeCompressed(const quint64 &value);
    quint64   sizeUniqueTotal() const;
    void      setSizeUniqueTotal(const quint64 &value);
    quint64   sizeUniqueCompressed() const;
    void      setSizeUniqueCompressed(const quint64 &value);
    QString   command() const;
    void      setCommand(const QString &value);
    QString   contents() const;
    void      setContents(const QString &value);
    QString   jobRef() const;
    void      setJobRef(const QString &jobRef);
    //! @}

    //! Returns whether the tarsnap command included "-P" (preserve pathnames).
    bool hasPreservePaths();
    //! Returns whether this Archive has been scheduled for deletion.
    bool deleteScheduled() const;
    //! Sets whether this Archive is scheduled for deletion. Emits changed().
    void setDeleteScheduled(bool deleteScheduled);

    // From PersistentObject
    //! Saves this object to the PersistentStore; creating or
    //! updating as appropriate.
    void save() override;
    //! Loads this object from the PersistentStore.  The object's
    //! \c _name must already be set.
    void load() override;
    //! Deletes this object from the PersistentStore.  The object's
    //! \c _name must already be set.
    void purge() override;
    //! Returns whether an object with this key exists in the PersistentStore.
    bool doesKeyExist(const QString &key) override;

public slots:
    //! Returns statistics about this archive.
    QString archiveStats();

signals:
    //! This item was saved.
    void changed();
    //! This item was deleted.
    void purged();

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

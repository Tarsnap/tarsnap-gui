#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "persistentobject.h"

#include <QDateTime>
#include <QObject>
#include <QSharedPointer>
#include <QStringList>

struct ArchiveRestoreOptions
{
    bool    optionRestore     = false;
    bool    optionRestoreDir  = false;
    bool    optionDownArchive = false;
    bool    overwriteFiles    = false;
    bool    keepNewerFiles    = true;
    QString path;
};

class Archive;
typedef QSharedPointer<Archive> ArchivePtr;

Q_DECLARE_METATYPE(ArchivePtr)

class Archive : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    explicit Archive(QObject *parent = 0);
    ~Archive();

    QString name() const;
    void setName(const QString &value);
    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &value);
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

    // From PersistentObject
    void save();
    void load();
    void purge();
    bool findObjectWithKey(QString key);

signals:
    void changed();
    void purged();

public slots:
    QString archiveStats();

private:
    QString     _name;
    QDateTime   _timestamp;
    quint64     _sizeTotal;
    quint64     _sizeCompressed;
    quint64     _sizeUniqueTotal;
    quint64     _sizeUniqueCompressed;
    QString     _command;
    QByteArray  _contents;
    QString     _jobRef;
};

#endif // ARCHIVE_H

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
    bool    optionRestore     = false;
    bool    optionRestoreDir  = false;
    bool    optionDownArchive = false;
    bool    overwriteFiles    = false;
    bool    keepNewerFiles    = true;
    QString     path;
    QStringList files;
};

class Archive;
typedef QSharedPointer<Archive> ArchivePtr;

Q_DECLARE_METATYPE(ArchivePtr)

struct File {
    QString name;
    QString modified;
    quint64 size;
    QString user;
    QString group;
    QString mode;
    quint64 links;
};

class ParseArchiveListingTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit ParseArchiveListingTask(const QString &listing):_listing(listing){}
    void run();

signals:
    void result(QVector<File> files);

private:
    QString _listing;
};

class Archive : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    explicit Archive(QObject *parent = nullptr);
    ~Archive();

    QString name() const;
    void setName(const QString &value);
    QDateTime timestamp() const;
    void setTimestamp(const QDateTime &value);
    bool truncated() const;
    void setTruncated(bool truncated);
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
    void getFileList();

    // From PersistentObject
    void save();
    void load();
    void purge();
    bool findObjectWithKey(QString key);

public slots:
    QString archiveStats();

signals:
    void changed();
    void purged();
    void fileList(QVector<File> files);

private:
    QString    _name;
    QDateTime  _timestamp;
    bool       _truncated;
    quint64    _sizeTotal;
    quint64    _sizeCompressed;
    quint64    _sizeUniqueTotal;
    quint64    _sizeUniqueCompressed;
    QString    _command;
    QByteArray _contents;
    QString    _jobRef;
};

#endif // ARCHIVE_H

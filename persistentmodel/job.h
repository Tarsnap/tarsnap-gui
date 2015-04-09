#ifndef JOB_H
#define JOB_H

#include "persistentmodel/archive.h"

#include <QObject>
#include <QUuid>
#include <QUrl>

class Job;
typedef QSharedPointer<Job> JobPtr;

Q_DECLARE_METATYPE(JobPtr)

class Job : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    explicit Job(QObject *parent = 0);
    ~Job();

    QString name() const;
    void setName(const QString &name);

    QList<QUrl> urls() const;
    void setUrls(const QList<QUrl> &urls);

    QList<ArchivePtr> archives() const;
    void setArchives(const QList<ArchivePtr> &archives);

    QUuid uuid() const;
    void setUuid(const QUuid &uuid);

    // From PersistentObject
    void save();
    void load();
    void purge();
    bool findObjectWithKey(QString key);

signals:

public slots:

private:
    QUuid               _uuid;
    QString             _name;
    QList<QUrl>         _urls;
    QList<ArchivePtr>   _archives;
};

#endif // JOB_H

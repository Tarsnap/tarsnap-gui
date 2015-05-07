#ifndef JOB_H
#define JOB_H

#include "persistentmodel/archive.h"

#include <QObject>
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

    bool optionPreservePaths() const;
    void setOptionPreservePaths(bool optionPreservePaths);

    // From PersistentObject
    void save();
    void load();
    void purge();
    bool findObjectWithKey(QString key);

signals:
    void changed();

public slots:
    void loadArchives();

private:
    QString             _name;
    QList<QUrl>         _urls;
    QList<ArchivePtr>   _archives;
    bool                _optionPreservePaths;
};

#endif // JOB_H

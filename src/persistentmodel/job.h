#ifndef JOB_H
#define JOB_H

#include "persistentmodel/archive.h"
#include "backuptask.h"

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

    bool optionScheduledEnabled() const;
    void setOptionScheduledEnabled(bool optionScheduledEnabled);

    bool optionPreservePaths() const;
    void setOptionPreservePaths(bool optionPreservePaths);

    bool optionTraverseMount() const;
    void setOptionTraverseMount(bool optionTraverseMount);

    bool optionFollowSymLinks() const;
    void setOptionFollowSymLinks(bool optionFollowSymLinks);

    quint64 optionSkipFilesSize() const;
    void setOptionSkipFilesSize(const quint64 &optionSkipFilesSize);

    BackupTaskPtr createBackupTask();

    // From PersistentObject
    void save();
    void load();
    void purge();
    bool findObjectWithKey(QString key);

signals:
    void changed();
    void loadArchives();

public slots:
    void backupTaskUpdate(const TaskStatus &status);

private:
    QString             _name;
    QList<QUrl>         _urls;
    QList<ArchivePtr>   _archives;
    bool                _optionScheduledEnabled;
    bool                _optionPreservePaths;
    bool                _optionTraverseMount;
    bool                _optionFollowSymLinks;
    quint64             _optionSkipFilesSize;
};

#endif // JOB_H

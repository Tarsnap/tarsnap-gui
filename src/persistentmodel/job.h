#ifndef JOB_H
#define JOB_H

#include "backuptask.h"
#include "persistentmodel/archive.h"

#include <QFileSystemWatcher>
#include <QObject>
#include <QUrl>

#define JOB_NAME_PREFIX QLatin1String("Job_")

class Job;
typedef QSharedPointer<Job> JobPtr;

Q_DECLARE_METATYPE(JobPtr)

class Job : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    explicit Job(QObject *parent = nullptr);
    ~Job();

    QString name() const;
    void setName(const QString &name);

    QString archivePrefix();

    QList<QUrl> urls() const;
    void setUrls(const QList<QUrl> &urls);
    bool validateUrls();
    void installWatcher();
    void removeWatcher();

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

    int optionSkipFilesSize() const;
    void setOptionSkipFilesSize(const int &optionSkipFilesSize);

    bool optionSkipFiles() const;
    void setOptionSkipFiles(bool optionSkipFiles);

    QString optionSkipFilesPatterns() const;
    void setOptionSkipFilesPatterns(const QString &optionSkipFilesPatterns);

    bool optionSkipNoDump() const;
    void setOptionSkipNoDump(bool optionSkipNoDump);

    bool settingShowHidden() const;
    void setSettingShowHidden(bool settingShowHidden);

    bool settingShowSystem() const;
    void setSettingShowSystem(bool settingShowSystem);

    bool settingHideSymlinks() const;
    void setSettingHideSymlinks(bool settingHideSymlinks);

    BackupTaskPtr createBackupTask();

    // From PersistentObject
    void save();
    void load();
    void purge();
    bool doesKeyExist(QString key);

signals:
    void changed();
    void loadArchives();
    void fsEvent();

private:
    QString            _name;
    QList<QUrl>        _urls;
    QList<ArchivePtr>  _archives;
    QFileSystemWatcher _fsWatcher;
    bool               _optionScheduledEnabled;
    bool               _optionPreservePaths;
    bool               _optionTraverseMount;
    bool               _optionFollowSymLinks;
    int                _optionSkipFilesSize;
    bool               _optionSkipFiles;
    QString            _optionSkipFilesPatterns;
    bool               _optionSkipNoDump;
    bool               _settingShowHidden;
    bool               _settingShowSystem;
    bool               _settingHideSymlinks;
};

#endif // JOB_H

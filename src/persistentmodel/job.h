#ifndef JOB_H
#define JOB_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QFileSystemWatcher>
#include <QObject>
#include <QUrl>
WARNINGS_ENABLE

#include "backuptask.h"
#include "persistentmodel/archive.h"

#define JOB_NAME_PREFIX QLatin1String("Job_")

class Job;
typedef QSharedPointer<Job> JobPtr;

Q_DECLARE_METATYPE(JobPtr)

typedef enum
{
    Disabled,
    Daily,
    Weekly,
    Monthly
} JobSchedule;

/*!
 * \ingroup persistent
 * \brief The Job stores metadata about a user's scheduled job.
 */
class Job : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    //! Constructor.
    explicit Job(QObject *parent = nullptr);
    ~Job() override;

    //! Returns JOB_NAME_PREFIX + job name.
    QString archivePrefix();

    //! Checks that each file listed in the Job exists.
    bool validateUrls();

    //! Installs a watcher to notify us if any files or
    //! directories in this Job are changed.
    void installWatcher();
    //! Removes the filesystem watcher.
    void removeWatcher();

    //! \name Getter/setter methods
    //! @{
    QString name() const;
    void    setName(const QString &name);

    QList<QUrl> urls() const;
    void        setUrls(const QList<QUrl> &urls);

    QList<ArchivePtr> archives() const;
    void              setArchives(const QList<ArchivePtr> &archives);

    JobSchedule optionScheduledEnabled() const;
    void        setOptionScheduledEnabled(JobSchedule schedule);

    bool optionPreservePaths() const;
    void setOptionPreservePaths(bool optionPreservePaths);

    bool optionTraverseMount() const;
    void setOptionTraverseMount(bool optionTraverseMount);

    bool optionFollowSymLinks() const;
    void setOptionFollowSymLinks(bool optionFollowSymLinks);

    int  optionSkipFilesSize() const;
    void setOptionSkipFilesSize(const int &optionSkipFilesSize);

    bool optionSkipFiles() const;
    void setOptionSkipFiles(bool optionSkipFiles);

    QString optionSkipFilesPatterns() const;
    void    setOptionSkipFilesPatterns(const QString &optionSkipFilesPatterns);

    bool optionSkipNoDump() const;
    void setOptionSkipNoDump(bool optionSkipNoDump);

    bool settingShowHidden() const;
    void setSettingShowHidden(bool settingShowHidden);

    bool settingShowSystem() const;
    void setSettingShowSystem(bool settingShowSystem);

    bool settingHideSymlinks() const;
    void setSettingHideSymlinks(bool settingHideSymlinks);
    //! @}

    //! Create a backupTask() which may be passed to TaskManager::backupNow().
    BackupTaskPtr createBackupTask();

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
    bool doesKeyExist(QString key) override;

signals:
    //! The list of archives belonging to this backup has changed.
    void changed();
    //! This Job was deleted.
    void purged();
    //! Notifies that the list of archives belonging to this Job
    //! needs to be refreshed.
    void loadArchives();
    //! A file or directory (which is being watched) has changed.
    void fsEvent();

private:
    QString            _name;
    QList<QUrl>        _urls;
    QList<ArchivePtr>  _archives;
    QFileSystemWatcher _fsWatcher;
    int                _optionScheduledEnabled;
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

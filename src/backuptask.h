#ifndef BACKUPTASK_H
#define BACKUPTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
WARNINGS_ENABLE

#include <persistentmodel/archive.h>

class BackupTaskData;

typedef QSharedPointer<BackupTaskData> BackupTaskDataPtr;
Q_DECLARE_METATYPE(BackupTaskDataPtr)

/*!
 * \ingroup background-tasks
 * \brief The BackupTaskData is a QObject which stores settings for making a
 * new Archive.
 */
class BackupTaskData : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    BackupTaskData();
    ~BackupTaskData() {}

    //! Make the list of --exclude files and dirs.
    QStringList getExcludesList();

    //! Getter/setter methods
    //! @{
    QString name() const;
    void    setName(const QString &name);

    QUuid uuid() const;
    void  setUuid(const QUuid &uuid);

    QDateTime timestamp() const;
    void      setTimestamp(const QDateTime &timestamp);

    QString jobRef() const;
    void    setJobRef(const QString &jobRef);

    QList<QUrl> urls() const;
    void        setUrls(const QList<QUrl> &urls);

    bool optionPreservePaths() const;
    void setOptionPreservePaths(bool optionPreservePaths);

    bool optionTraverseMount() const;
    void setOptionTraverseMount(bool optionTraverseMount);

    bool optionFollowSymLinks() const;
    void setOptionFollowSymLinks(bool optionFollowSymLinks);

    quint64 optionSkipFilesSize() const;
    void    setOptionSkipFilesSize(const int &optionSkipFilesSize);

    bool optionSkipSystem() const;
    void setOptionSkipSystem(bool optionSkipSystem);

    QStringList optionSkipSystemFiles() const;
    void setOptionSkipSystemFiles(const QStringList &optionSkipSystemFiles);
    void setOptionSkipSystemFiles(const QString string);

    bool optionDryRun() const;
    void setOptionDryRun(bool optionDryRun);

    bool optionSkipNoDump() const;
    void setOptionSkipNoDump(bool optionSkipNoDump);

    int  exitCode() const;
    void setExitCode(int exitCode);

    QString output() const;
    void    setOutput(const QString &output);

    ArchivePtr archive() const;
    void       setArchive(const ArchivePtr &archive);

    QString command() const;
    void    setCommand(const QString &command);
    //! @}

private:
    QString     _name;
    QUuid       _uuid;
    QDateTime   _timestamp;
    QString     _jobRef;
    QList<QUrl> _urls;
    bool        _optionPreservePaths;
    bool        _optionTraverseMount;
    bool        _optionFollowSymLinks;
    quint64     _optionSkipFilesSize;
    bool        _optionSkipSystem;
    QStringList _optionSkipSystemFiles;
    bool        _optionDryRun;
    bool        _optionSkipNoDump;

    int        _exitCode;
    QString    _output;
    ArchivePtr _archive;
    QString    _command;
};

#endif // BACKUPTASK_H

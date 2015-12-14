#ifndef BACKUPTASK_H
#define BACKUPTASK_H

#include <persistentmodel/archive.h>

#include <QObject>

enum TaskStatus { Queued, Running, Completed, Failed, Paused, Initialized };

class BackupTask;

typedef BackupTask* BackupTaskPtr;

class BackupTask: public QObject
{
    Q_OBJECT

public:
    BackupTask();
    ~BackupTask() {}

    QUuid uuid() const {return _uuid;}
    void setUuid(const QUuid &uuid) {_uuid = uuid;}

    QDateTime timestamp() const {return _timestamp;}
    void updateTimestamp() {_timestamp = QDateTime::currentDateTime();}

    QList<QUrl> urls() const {return _urls;}
    void setUrls(const QList<QUrl> &urls) {_urls = urls;}

    QString name() const {return _name;}
    void setName(const QString &name) {_name = name;}

    TaskStatus status() const {return _status;}
    void setStatus(const TaskStatus &status) {_status = status; emit statusUpdate(_status);}

    int exitCode() const {return _exitCode;}
    void setExitCode(int exitCode) {_exitCode = exitCode;}

    QString output() const {return _output;}
    void setOutput(const QString &output) {_output = output;}

    ArchivePtr archive() const {return _archive;}
    void setArchive(const ArchivePtr &archive) {_archive = archive;}

    QString jobRef() const {return _jobRef;}
    void setJobRef(const QString &job) {_jobRef = job;}

    bool optionPreservePaths() const;
    void setOptionPreservePaths(bool optionPreservePaths);

    bool optionTraverseMount() const;
    void setOptionTraverseMount(bool optionTraverseMount);

    bool optionFollowSymLinks() const;
    void setOptionFollowSymLinks(bool optionFollowSymLinks);

    quint64 optionSkipFilesSize() const;
    void setOptionSkipFilesSize(const quint64 &optionSkipFilesSize);

    bool optionSkipSystem() const;
    void setOptionSkipSystem(bool optionSkipSystem);

    QStringList optionSkipSystemFiles() const;
    void setOptionSkipSystemFiles(const QStringList &optionSkipSystemFiles);
    void setOptionSkipSystemFiles(const QString string);

    QStringList getExcludesList();

    bool optionDryRun() const;
    void setOptionDryRun(bool optionDryRun);

    bool optionNoDump() const;
    void setOptionNoDump(bool optionNoDump);

signals:
    void statusUpdate(const TaskStatus &status);

private:
    QUuid                 _uuid;
    QDateTime             _timestamp;
    QString               _jobRef;
    QList<QUrl>           _urls;
    QString               _name;
    bool                  _optionPreservePaths;
    bool                  _optionTraverseMount;
    bool                  _optionFollowSymLinks;
    quint64               _optionSkipFilesSize;
    bool                  _optionSkipSystem;
    QStringList           _optionSkipSystemFiles;
    bool                  _optionDryRun;
    bool                  _optionNoDump;

    TaskStatus            _status;
    int                   _exitCode;
    QString               _output;
    ArchivePtr            _archive;
};

#endif // BACKUPTASK_H

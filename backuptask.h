#ifndef BACKUPTASK_H
#define BACKUPTASK_H

#include <persistentmodel/archive.h>
#include <persistentmodel/job.h>

#include <QObject>

enum TaskStatus { Queued, Running, Completed, Failed, Paused, Initialized };

class BackupTask;

typedef BackupTask* BackupTaskPtr;

class BackupTask: public QObject
{
    Q_OBJECT

public:
    BackupTask();
    ~BackupTask();

    QUuid uuid() const {return _uuid;}
    void setUuid(const QUuid &uuid) {_uuid = uuid;}

    QList<QUrl> urls() const {return _urls;}
    void setUrls(const QList<QUrl> &urls) {_urls = urls;}

    QString name() const {return _name;}
    void setName(const QString &name) {_name = name;}

    TaskStatus status() const {return _status;}
    void setStatus(const TaskStatus &status) {_status = status; emit statusUpdate();}

    int exitCode() const {return _exitCode;}
    void setExitCode(int exitCode) {_exitCode = exitCode;}

    QString output() const {return _output;}
    void setOutput(const QString &output) {_output = output;}

    ArchivePtr archive() const {return _archive;}
    void setArchive(const ArchivePtr &archive) {_archive = archive;}

    JobPtr job() const {return _job;}
    void setJob(const JobPtr &job) {_job = job;}

    bool optionPreservePaths() const {return _optionPreservePaths;}
    void setOptionPreservePaths(bool optionPreservePaths) {_optionPreservePaths = optionPreservePaths;}

    QStringList getExcludesList();

signals:
    void statusUpdate();

private:
    QUuid                 _uuid;
    QList<QUrl>           _urls;
    QString               _name;
    bool                  _optionPreservePaths;
    qint64                _skipFilesSize;

    TaskStatus            _status;
    int                   _exitCode;
    QString               _output;
    ArchivePtr            _archive;
    JobPtr                _job;
};

#endif // BACKUPTASK_H

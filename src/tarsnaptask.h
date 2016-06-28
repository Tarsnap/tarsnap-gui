#ifndef TARSNAPTASK_H
#define TARSNAPTASK_H

#include <QEventLoopLocker>
#include <QProcess>
#include <QRunnable>
#include <QThread>
#include <QVariant>

class TarsnapTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit TarsnapTask();
    ~TarsnapTask();

    void run();
    void stop(bool kill = false);
    void interrupt();
    bool waitForTask();
    QProcess::ProcessState taskStatus();

    QString command() const;
    void setCommand(const QString &command);

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);

    QString password() const;
    void setPassword(const QString &password);

    bool requiresPassword() const;
    void setRequiresPassword(bool requiresPassword);

    void setStandardOutputFile(const QString &fileName);

    QVariant data() const;
    void setData(const QVariant &data);

    bool truncateLogOutput() const;
    void setTruncateLogOutput(bool truncateLogOutput);

signals:
    void finished(QVariant data, int exitCode, QString output);
    void started(QVariant data);
    void terminated();

private slots:
    void readProcessOutput();
    void processFinished();
    void processError();

private:
    QVariant    _data; // caller supplied data
    QProcess   *_process;
    QByteArray  _processOutput;
    QString     _command;
    QStringList _arguments;
    QString     _password;
    bool        _requiresPassword;
    QString     _standardOutFile;
    bool        _truncateLogOutput;
    QEventLoopLocker _lock;
};

#endif // TARSNAPTASK_H

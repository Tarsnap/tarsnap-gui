#ifndef TARSNAPTASK_H
#define TARSNAPTASK_H

#include <QEventLoopLocker>
#include <QProcess>
#include <QRunnable>
#include <QThread>
#include <QUuid>
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
    void cancel();
    bool waitForTask();
    QProcess::ProcessState taskStatus();

    QString command() const;
    void setCommand(const QString &command);

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);

    void setStdIn(const QString &stdIn);
    void setStdOutFile(const QString &fileName);

    QVariant data() const;
    void setData(const QVariant &data);

    bool truncateLogOutput() const;
    void setTruncateLogOutput(bool truncateLogOutput);

signals:
    void started(QVariant data);
    void finished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    void canceled(QVariant data);
    void dequeue();

private slots:
    void readProcessOutput();
    void processFinished();
    void processError();

private:
    QUuid       _id;
    QVariant    _data; // caller supplied data
    QProcess   *_process;
    QByteArray  _stdOut;
    QByteArray  _stdErr;
    QString     _stdIn;
    QString     _stdOutFile;
    QString     _command;
    QStringList _arguments;
    bool        _truncateLogOutput;
    QEventLoopLocker _lock;
};

#endif // TARSNAPTASK_H

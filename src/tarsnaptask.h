#ifndef TARSNAPTASK_H
#define TARSNAPTASK_H

#include <QEventLoopLocker>
#include <QProcess>
#include <QRunnable>
#include <QThread>
#include <QUuid>
#include <QVariant>

/*!
 * \ingroup background-tasks
 * \brief The TarsnapTask is a QObject and QRunnable which executes a
 * command-line command.
 */
class TarsnapTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    //! Constructor.
    explicit TarsnapTask();
    ~TarsnapTask();

    //! Run the command previously given.  Blocks until completed (or
    //! failed).
    void run();
    //! If the QProcess is running, attempt to stop it with
    //! QProcess::terminate().
    //! \param kill: if QProcess:terminate() fails, use QProcess::kill().
    void stop(bool kill = false);
    //! Send the QProcess a SIGQUIT.
    //! \warning MacOS X only.  (?)
    void interrupt();
    //! Emits \ref canceled.
    //! \warning Does not actually cancel a running task!
    void cancel();
    //! Blocks until the QProcess is finished.
    bool waitForTask();

    //! Gets the state of the QProcess.
    QProcess::ProcessState taskStatus();

    //! \name Getter/setter methods
    //! @{
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
    //! @}

signals:
    //! Started running the QProcess.
    void started(QVariant data);
    //! Finished running the QProcess.
    //! \warning Is not emitted if the process crashed.
    void finished(QVariant data, int exitCode, QString stdOut, QString stdErr);
    //! The QProcess was canceled.
    void canceled(QVariant data);
    //! The QProcess failed to start, or finished (either with success or
    //! failure.
    void dequeue();

private slots:
    void readProcessOutput();
    void processFinished();
    void processError();

private:
    QUuid            _id;
    QVariant         _data; // caller supplied data
    QProcess *       _process;
    QByteArray       _stdOut;
    QByteArray       _stdErr;
    QString          _stdIn;
    QString          _stdOutFile;
    QString          _command;
    QStringList      _arguments;
    bool             _truncateLogOutput;
    QEventLoopLocker _lock;
};

#endif // TARSNAPTASK_H

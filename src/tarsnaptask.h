#ifndef TARSNAPTASK_H
#define TARSNAPTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QProcess>
#include <QRunnable>
#include <QUuid>
#include <QVariant>
WARNINGS_ENABLE

/*
 * Normal exit codes are non-negative, but since we're just passing around
 * an int, we might as well use negative values for special meanings.
 */
#define EXIT_NO_MEANING (-1)
#define EXIT_CRASHED (-2)
#define EXIT_DID_NOT_START (-3)
#define EXIT_CMD_NOT_FOUND (-4)
#define EXIT_FAKE_REQUEST (-5)

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

#ifdef QT_TESTLIB_LIB
    void fake();
#endif

    //! Run the command previously given.  Blocks until completed (or
    //! failed).
    void run();
    //! If the QProcess is running, attempt to stop it with
    //! QProcess::terminate().
    //! \param kill if QProcess:terminate() fails, use QProcess::kill().
    void stop(bool kill = false);
    //! Send the QProcess a SIGQUIT.
    void sigquit();
    //! Emits \ref canceled.
    //! \warning Does not actually cancel a running task!
    void cancel();

    //! \name Getter/setter methods
    //! @{
    QString command() const;
    void    setCommand(const QString &command);

    QStringList arguments() const;
    void        setArguments(const QStringList &arguments);

    void setStdIn(const QString &stdIn);
    void setStdOutFile(const QString &fileName);

    QVariant data() const;
    void     setData(const QVariant &data);

    void setTruncateLogOutput(bool truncateLogOutput);
    //! @}

signals:
    //! Started running the QProcess.
    void started(QVariant data);
    //! Finished, crashed, or could not start running the QProcess.
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
    // Housekeeping.
    QUuid     _uuid;
    QVariant  _data; // caller supplied data
    QProcess *_process;

    // Standard POSIX input/output.
    QByteArray _stdOut;
    QByteArray _stdErr;
    QByteArray _stdIn;
    int        _exitCode;

    // Influences standard output.
    QString _stdOutFilename;
    bool    _truncateLogOutput;

    // Actual command.
    QString     _command;
    QStringList _arguments;

    // Utility function.
    QByteArray truncate_output(QByteArray stdOut);

#ifdef QT_TESTLIB_LIB
    bool _fake;
#endif
};

#endif // TARSNAPTASK_H

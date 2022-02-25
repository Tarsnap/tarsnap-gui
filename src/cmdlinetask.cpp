#include "cmdlinetask.h"

WARNINGS_DISABLE
#include <QChar>
#include <QProcess>
#include <QRegExp>
#include <QStandardPaths>
#include <QUuid>
WARNINGS_ENABLE

#include "ConsoleLog.h"

#include <signal.h>

#define DEFAULT_TIMEOUT_MS 5000
#define LOG_MAX_LENGTH 3072
#define LOG_MAX_SEARCH_NL 1024

static const QString quoteCommandLine(const QStringList &args)
{
    QStringList escaped;
    QRegExp     rx("^[0-9a-z-A-Z/._-]*$");
    QString     cmdLine;

    for(int i = 0; i < args.size(); ++i)
    {
        QString arg = args.at(i);
        if(rx.indexIn(arg) >= 0)
        {
            escaped.append(arg);
        }
        else
        {
            escaped.append(arg.prepend("\'").append("\'"));
        }
    }

    cmdLine = escaped.join(' ');
    return (cmdLine);
}

CmdlineTask::CmdlineTask()
    : BaseTask(),
      _process(nullptr),
      _exitCode(EXIT_NO_MEANING),
      _truncateLogOutput(false),
      _monitorOutput(false)
{
}

void CmdlineTask::run()
{
    bool finishedStatus = false;

    Q_ASSERT(_process == nullptr);

    // Set up new _process
    _process = new QProcess();
    _process->setProgram(_command);
    _process->setArguments(_arguments);
    if(!_stdOutFilename.isEmpty())
        _process->setStandardOutputFile(_stdOutFilename);

    LOG << tr("Task %1 started:\n[%2 %3]\n")
               .arg(_uuid.toString())
               .arg(_process->program())
               .arg(quoteCommandLine(_process->arguments()));

#ifdef QT_TESTLIB_LIB
    // Bail if desired.  Make sure this happens before we check if the
    // command exists, otherwise we might bail for the wrong reason.
    if(_fake)
    {
        LOG << "Not running task due to 'fake this task' request.\n";
        _fake = false;
        emit started(_data);
        emit finished(_data, EXIT_FAKE_REQUEST, "", "");
        goto cleanup;
    }
#endif

    // Make sure that the command exists and is executable, because QProcess
    // doesn't clean up its memory if it fails due to "command not found".
    if(QStandardPaths::findExecutable(_command).isEmpty())
    {
        LOG << QString("Command '%1' not found\n").arg(_command);
        emit finished(_data, EXIT_CMD_NOT_FOUND, "", "");
        goto cleanup;
    }

    // Read from stdout
    if(_monitorOutput)
    {
        connect(_process, &QProcess::readyReadStandardOutput, this,
                &CmdlineTask::gotStdout);
    }

    // Start the _process, and wait for confirmation of it starting.
    _process->start();
    if(_process->waitForStarted(DEFAULT_TIMEOUT_MS))
    {
        emit started(_data);
    }
    else
    {
        _exitCode = EXIT_DID_NOT_START;
        processError(_process);
        goto cleanup;
    }

    // Write to the process' stdin.
    if(!_stdIn.isEmpty())
    {
        _process->write(_stdIn.data(), _stdIn.size());
        _process->closeWriteChannel();
    }

    // Wait indefinitely for the process to finish
    finishedStatus = _process->waitForFinished(-1);

    // Cancel monitoring output
    if(_monitorOutput)
    {
        disconnect(_process, &QProcess::readyReadStandardOutput, this, nullptr);
    }

    // Deal with the "finished" status
    if(finishedStatus)
    {
        readProcessOutput(_process);
        processFinished(_process);
    }
    else
    {
        readProcessOutput(_process);
        processError(_process);
        goto cleanup;
    }

cleanup:
    delete _process;
    _process = nullptr;
    emit dequeue();
}

void CmdlineTask::stop()
{
    // Bail if the TaskManager has recorded this as "started" but it
    // hasn't actually begun yet.  See taskqueuer.cpp for the explanation.
    if(_process == nullptr)
        return;

    if(_process->state() == QProcess::Running)
        _process->terminate();
}

void CmdlineTask::gotStdout()
{
    Q_ASSERT(_process != nullptr);
    emit outputStdout(_process->readAllStandardOutput());
}

void CmdlineTask::sigquit()
{
    // Bail if the TaskManager has recorded this as "started" but it
    // hasn't actually begun yet.  See taskqueuer.cpp for the explanation.
    if(_process == nullptr)
        return;

#if(QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    // Assume that qint64 is a valid pid_t on this system.
    pid_t pid = static_cast<pid_t>(_process->processId());
#else
    // Assume that Q_PID is a valid pid on this system.
    pid_t pid = static_cast<pid_t>(_process->pid());
#endif

    // If this is sent to the Tarsnap client creating an archive, it will
    // truncate it and leave a '.part' partial archive.
    kill(pid, SIGQUIT);
}

QString CmdlineTask::command() const
{
    return _command;
}

void CmdlineTask::setCommand(const QString &command)
{
    _command = command;
}

QStringList CmdlineTask::arguments() const
{
    return _arguments;
}

void CmdlineTask::setArguments(const QStringList &arguments)
{
    _arguments = arguments;
}

void CmdlineTask::setStdIn(const QString &standardIn)
{
    _stdIn = QByteArray(standardIn.toUtf8());
}

void CmdlineTask::setStdOutFile(const QString &fileName)
{
    _stdOutFilename = fileName;
}

QVariant CmdlineTask::data() const
{
    return _data;
}

void CmdlineTask::setData(const QVariant &data)
{
    _data = data;
}

void CmdlineTask::setTruncateLogOutput(bool truncateLogOutput)
{
    _truncateLogOutput = truncateLogOutput;
}

void CmdlineTask::setMonitorOutput()
{
    _monitorOutput = true;
}

void CmdlineTask::readProcessOutput(QProcess *process)
{
    if(_stdOutFilename.isEmpty())
        _stdOut.append(process->readAllStandardOutput().trimmed());
    _stdErr.append(process->readAllStandardError().trimmed());
}

QByteArray CmdlineTask::truncate_output(QByteArray stdOutArray)
{
    QString stdOut = QString(stdOutArray);
    // Find a good newline to which to truncate.
    int from = LOG_MAX_LENGTH
               + qMin(stdOut.size() - LOG_MAX_LENGTH, LOG_MAX_SEARCH_NL);
    int nextNL = stdOut.lastIndexOf(QChar('\n'), from);
    // Only keep the first part of the logfile.
    stdOut.truncate(qMax(LOG_MAX_LENGTH, nextNL));
    // Notify about truncation in log.
    int num_truncated = _stdOut.mid(stdOut.size()).count('\n');
    stdOut.append(tr("\n...\n-- %1 output lines truncated by Tarsnap GUI --\n")
                      .arg(num_truncated));
    return stdOut.toUtf8();
}

void CmdlineTask::processFinished(QProcess *process)
{
    switch(process->exitStatus())
    {
    case QProcess::NormalExit:
    {
        _exitCode = process->exitCode();
        emit finished(_data, _exitCode, QString(_stdOut), QString(_stdErr));

        // Truncate LOG output
        QByteArray stdOut(_stdOut);
        if(_truncateLogOutput && (stdOut.size() > LOG_MAX_LENGTH))
            stdOut = truncate_output(stdOut);

        LOG << tr("Task %1 finished with exit code %2:\n[%3 %4]\n%5\n")
                   .arg(_uuid.toString())
                   .arg(_exitCode)
                   .arg(_command)
                   .arg(quoteCommandLine(_arguments))
                   .arg(QString(stdOut + _stdErr));
        break;
    }
    case QProcess::CrashExit:
    {
        _exitCode = EXIT_CRASHED;
        processError(process);
        break;
    }
    }
}

void CmdlineTask::processError(QProcess *process)
{
    LOG << tr("Task %1 finished with error %2 (%3) occured "
              "(exit code %4):\n[%5 %6]\n%7\n")
               .arg(_uuid.toString())
               .arg(process->error())
               .arg(process->errorString())
               .arg(_exitCode)
               .arg(_command)
               .arg(quoteCommandLine(_arguments))
               .arg(QString(_stdOut + _stdErr).trimmed());
    emit finished(_data, _exitCode, QString(_stdOut), QString(_stdErr));
    emit canceled();
}

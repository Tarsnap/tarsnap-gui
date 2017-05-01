#include "tarsnaptask.h"
#include "utils.h"
#include "debug.h"

#if defined Q_OS_UNIX
#include <signal.h>
#endif

#define DEFAULT_TIMEOUT_MS 5000
#define LOG_MAX_LENGTH 3072
#define LOG_MAX_SEARCH_NL 1024

TarsnapTask::TarsnapTask()
    : QObject(), _id(QUuid::createUuid()), _process(nullptr),
      _truncateLogOutput(false)
{
}

TarsnapTask::~TarsnapTask()
{
}

void TarsnapTask::run()
{
    _process = new QProcess();
    _process->setProgram(_command);
    _process->setArguments(_arguments);
    if(!_stdOutFile.isEmpty())
        _process->setStandardOutputFile(_stdOutFile);

    LOG << tr("Executing task %1:\n[%2 %3]")
               .arg(_id.toString())
               .arg(_process->program())
               .arg(Utils::quoteCommandLine(_process->arguments()));

    _process->start();

    if(_process->waitForStarted(DEFAULT_TIMEOUT_MS))
    {
        emit started(_data);
    }
    else
    {
        processError();
        goto cleanup;
    }

    if(!_stdIn.isEmpty())
    {
        QByteArray password(_stdIn.toUtf8());
        _process->write(password.data(), password.size());
        _process->closeWriteChannel();
    }

    if(_process->waitForFinished(-1))
    {
        readProcessOutput();
        processFinished();
    }
    else
    {
        readProcessOutput();
        processError();
        goto cleanup;
    }

cleanup:
    delete _process;
    _process = nullptr;
    emit dequeue();
}

void TarsnapTask::stop(bool kill)
{
    if(_process->state() == QProcess::Running)
    {
        _process->terminate();
        if(kill && (false == _process->waitForFinished(DEFAULT_TIMEOUT_MS)))
            _process->kill();
    }
}

void TarsnapTask::interrupt()
{
#if defined Q_OS_UNIX
    kill(_process->pid(), SIGQUIT);
#endif
}

void TarsnapTask::cancel()
{
    emit canceled(_data);
}

bool TarsnapTask::waitForTask()
{
    return _process->waitForFinished(-1);
}

QProcess::ProcessState TarsnapTask::taskStatus()
{
    return _process->state();
}

QString TarsnapTask::command() const
{
    return _command;
}

void TarsnapTask::setCommand(const QString &command)
{
    _command = command;
}

QStringList TarsnapTask::arguments() const
{
    return _arguments;
}

void TarsnapTask::setArguments(const QStringList &arguments)
{
    _arguments = arguments;
}

void TarsnapTask::setStdIn(const QString &standardIn)
{
    _stdIn = standardIn;
}

void TarsnapTask::setStdOutFile(const QString &fileName)
{
    _stdOutFile = fileName;
}

QVariant TarsnapTask::data() const
{
    return _data;
}

void TarsnapTask::setData(const QVariant &data)
{
    _data = data;
}

bool TarsnapTask::truncateLogOutput() const
{
    return _truncateLogOutput;
}

void TarsnapTask::setTruncateLogOutput(bool truncateLogOutput)
{
    _truncateLogOutput = truncateLogOutput;
}

void TarsnapTask::readProcessOutput()
{
    if(_stdOutFile.isEmpty())
        _stdOut.append(_process->readAllStandardOutput().trimmed());
    _stdErr.append(_process->readAllStandardError().trimmed());
}

void TarsnapTask::processFinished()
{
    switch(_process->exitStatus())
    {
    case QProcess::NormalExit:
    {
        emit finished(_data, _process->exitCode(), QString(_stdOut), QString(_stdErr));
        QByteArray stdOut(_stdOut);
        if(!stdOut.isEmpty() && _truncateLogOutput
           && (stdOut.size() > LOG_MAX_LENGTH))
        {
            int nextNL = stdOut.lastIndexOf(QChar('\n'),
                                            LOG_MAX_LENGTH +
                                            std::min(stdOut.size() - LOG_MAX_LENGTH,
                                                     LOG_MAX_SEARCH_NL));
            stdOut.truncate(std::max(LOG_MAX_LENGTH, nextNL));
            stdOut.append(tr("\n...\n-- %1 output lines truncated by Tarsnap GUI --\n")
                          .arg(_stdOut.mid(stdOut.size())
                               .count(QChar('\n').toLatin1())));
        }
        LOG << tr("Task %1 finished with exit code %2:\n[%3 %4]\n%5")
               .arg(_id.toString())
               .arg(_process->exitCode())
               .arg(_command)
               .arg(Utils::quoteCommandLine(_arguments))
               .arg(QString(stdOut + _stdErr));
        break;
    }
    case QProcess::CrashExit:
    {
        processError();
        break;
    }
    }
}

void TarsnapTask::processError()
{
    LOG << tr("Task %1 finished with error %2 (%3) occured (exit code %4):\n[%5 %6]\n%7")
               .arg(_id.toString())
               .arg(_process->error())
               .arg(_process->errorString())
               .arg(_process->exitCode())
               .arg(_command)
               .arg(Utils::quoteCommandLine(_arguments))
               .arg(QString(_stdOut + _stdErr).trimmed());
    cancel();
}

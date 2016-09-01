#include "tarsnaptask.h"
#include "debug.h"

#include <QSettings>

#if defined Q_OS_UNIX
#include <signal.h>
#endif

#define DEFAULT_TIMEOUT_MS 5000
#define LOG_OUTPUT_BYTES 10240

TarsnapTask::TarsnapTask()
    : QObject(), _process(nullptr), _truncateLogOutput(false)
{
}

TarsnapTask::~TarsnapTask()
{
}

void TarsnapTask::run()
{
    _process = new QProcess();
    if(_standardOutFile.isEmpty())
        _process->setProcessChannelMode(QProcess::MergedChannels);
    else
        _process->setStandardOutputFile(_standardOutFile);
    QSettings settings;
    int upload_rate_kbps = settings.value("app/limit_upload", 0).toInt();
    int download_rate_kbps = settings.value("app/limit_download", 0).toInt();
    if(download_rate_kbps)
    {
        _arguments.prepend("--maxbw-rate-down");
        _arguments.insert(1, QString::number(1024 * quint64(download_rate_kbps)));
    }
    if(upload_rate_kbps)
    {
        _arguments.prepend("--maxbw-rate-up");
        _arguments.insert(1, QString::number(1024 * quint64(upload_rate_kbps)));
    }
    if(settings.value("tarsnap/no_default_config", true).toBool())
        _arguments.prepend("--no-default-config");
    _process->setProgram(_command);
    _process->setArguments(_arguments);
    LOG << tr("Executing command:\n[%1 %2]")
               .arg(_process->program())
               .arg(_process->arguments().join(' '));
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
    if(!_standardIn.isEmpty())
    {
        QByteArray password(_standardIn.toUtf8());
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
        processError();
        goto cleanup;
    }
cleanup:
    delete _process;
    _process = nullptr;
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

QString TarsnapTask::standardIn() const
{
    return _standardIn;
}

void TarsnapTask::setStandardIn(const QString &standardIn)
{
    _standardIn = standardIn;
}

void TarsnapTask::setStandardOutputFile(const QString &fileName)
{
    _standardOutFile = fileName;
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
    if(_process->processChannelMode() == QProcess::MergedChannels)
        _processOutput.append(_process->readAll());
    else
        _processOutput.append(_process->readAllStandardError());
}

void TarsnapTask::processFinished()
{
    switch(_process->exitStatus())
    {
    case QProcess::NormalExit:
    {
        QString output(_processOutput);
        output = output.trimmed();
        emit finished(_data, _process->exitCode(), output);
        emit terminated();
        if(!output.isEmpty())
        {
            if(_truncateLogOutput)
            {
                output.truncate(LOG_OUTPUT_BYTES);
                output.append(tr("\n...\n-- Output truncated by Tarsnap GUI --"));
            }
            LOG << tr("Command finished with exit code %3 and output:\n[%1 %2]\n%4")
                       .arg(_command)
                       .arg(_arguments.join(' '))
                       .arg(_process->exitCode())
                       .arg(output);
        }
        else
            LOG << tr("Command finished with exit code %3 and no output:\n[%1 %2]")
                       .arg(_command)
                       .arg(_arguments.join(' '))
                       .arg(_process->exitCode());
    }
        break;
    case QProcess::CrashExit:
        processError();
        break;
    }
}

void TarsnapTask::processError()
{
    LOG << tr("Tarsnap process error %1 (%2) occured (exit code %3):\n%4")
               .arg(_process->error())
               .arg(_process->errorString())
               .arg(_process->exitCode())
               .arg(QString(_processOutput).trimmed());
    emit terminated();
}

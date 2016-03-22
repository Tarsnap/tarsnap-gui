#include "tarsnapclient.h"
#include "debug.h"

#include <QSettings>

#if defined Q_OS_UNIX
#include <signal.h>
#endif

#define DEFAULT_TIMEOUT_MS 5000

TarsnapClient::TarsnapClient()
    : QObject(), _process(NULL), _requiresPassword(false)
{
}

TarsnapClient::~TarsnapClient()
{
}

QString TarsnapClient::command() const
{
    return _command;
}

void TarsnapClient::setCommand(const QString &command)
{
    _command = command;
}

QStringList TarsnapClient::arguments() const
{
    return _arguments;
}

void TarsnapClient::setArguments(const QStringList &arguments)
{
    _arguments = arguments;
}

void TarsnapClient::run()
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
    if(settings.value("tarsnap/no_default_config", false).toBool())
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
    if(_requiresPassword)
    {
        QByteArray password(_password.toUtf8() + "\n");
        _process->write(password.data(), password.size());
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
    _process = NULL;
}

void TarsnapClient::stop(bool kill)
{
    if(_process->state() == QProcess::Running)
    {
        _process->terminate();
        if(kill && (false == _process->waitForFinished(DEFAULT_TIMEOUT_MS)))
            _process->kill();
    }
}

void TarsnapClient::interrupt()
{
#if defined Q_OS_UNIX
    kill(_process->pid(), SIGQUIT);
#endif
}

QProcess::ProcessState TarsnapClient::statusClient()
{
    return _process->state();
}

bool TarsnapClient::waitForClient()
{
    return _process->waitForFinished(-1);
}

void TarsnapClient::readProcessOutput()
{
    if(_process->processChannelMode() == QProcess::MergedChannels)
        _processOutput.append(_process->readAll());
    else
        _processOutput.append(_process->readAllStandardError());
}

void TarsnapClient::processFinished()
{
    QString output(_processOutput);
    switch(_process->exitStatus())
    {
    case QProcess::NormalExit:
        if(!output.isEmpty())
            LOG << tr("Command finished with exit code %3 and output:\n[%1 %2]\n%4")
                       .arg(_command)
                       .arg(_arguments.join(' '))
                       .arg(_process->exitCode())
                       .arg(output);
        else
            LOG << tr("Command finished with exit code %3 and no output:\n[%1 %2]")
                       .arg(_command)
                       .arg(_arguments.join(' '))
                       .arg(_process->exitCode());
        emit finished(_data, _process->exitCode(), output);
        emit terminated(_data);
        break;
    case QProcess::CrashExit:
        processError();
        break;
    }
}

void TarsnapClient::processError()
{
    LOG << tr("Tarsnap process error %1 (%2) occured (exit code %3):\n%4")
               .arg(_process->error())
               .arg(_process->errorString())
               .arg(_process->exitCode())
               .arg(QString(_processOutput));
    emit terminated(_data);
}
QVariant TarsnapClient::data() const
{
    return _data;
}

void TarsnapClient::setData(const QVariant &data)
{
    _data = data;
}

bool TarsnapClient::requiresPassword() const
{
    return _requiresPassword;
}

void TarsnapClient::setRequiresPassword(bool requiresPassword)
{
    _requiresPassword = requiresPassword;
}

void TarsnapClient::setStandardOutputFile(const QString &fileName)
{
    _standardOutFile = fileName;
}

QString TarsnapClient::password() const
{
    return _password;
}

void TarsnapClient::setPassword(const QString &password)
{
    _password = password;
}

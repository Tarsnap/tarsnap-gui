#include "tarsnapclient.h"
#include "debug.h"

#include <QSettings>

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
    _process->setProcessChannelMode(QProcess::MergedChannels);
    QSettings settings;
    if(settings.value("tarsnap/no_default_config", false).toBool())
        _arguments.prepend("--no-default-config");
    _process->setProgram(_command);
    _process->setArguments(_arguments);
    LOG << tr("Executing [%1 %2]")
               .arg(_process->program())
               .arg(_process->arguments().join(' '))
        << DELIMITER;
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
    _processOutput.append(_process->readAll());
}

void TarsnapClient::processFinished()
{
    QString output(_processOutput);
    switch(_process->exitStatus())
    {
    case QProcess::NormalExit:
        if(!output.isEmpty())
            LOG << tr("[%1 %2] finished with exit code %3 and output:\n%4")
                       .arg(_command)
                       .arg(_arguments.join(' '))
                       .arg(_process->exitCode())
                       .arg(output)
                << DELIMITER;
        else
            LOG << tr("[%1 %2] finished with exit code %3 and no output.")
                       .arg(_command)
                       .arg(_arguments.join(' '))
                       .arg(_process->exitCode())
                << DELIMITER;
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
               .arg(QString(_processOutput))
        << DELIMITER;
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

QString TarsnapClient::password() const
{
    return _password;
}

void TarsnapClient::setPassword(const QString &password)
{
    _password = password;
}

#include "tarsnapcli.h"

#include <QDebug>
#include <QEventLoop>

#define DEFAULT_TIMEOUT_MS 10000

TarsnapCLI::TarsnapCLI(QUuid uuid) : QObject(), _uuid(uuid), _process(NULL), _requiresPassword(false)
{
}

TarsnapCLI::~TarsnapCLI()
{
}
QString TarsnapCLI::command() const
{
    return _command;
}

void TarsnapCLI::setCommand(const QString &command)
{
    _command = command;
}
QStringList TarsnapCLI::arguments() const
{
    return _arguments;
}

void TarsnapCLI::setArguments(const QStringList &arguments)
{
    _arguments = arguments;
}

void TarsnapCLI::run()
{
    bool result = false;
    _process = new QProcess();
    _process->setProcessChannelMode( QProcess::MergedChannels );
//    connect(_process, SIGNAL(started()), this, SIGNAL(clientStarted()), Qt::QueuedConnection);
//    connect(_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()), Qt::QueuedConnection);
//    connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
//            SLOT(processFinished(int, QProcess::ExitStatus)), Qt::QueuedConnection);
//    connect(_process, SIGNAL(error(QProcess::ProcessError)), this,
//             SLOT(processError(QProcess::ProcessError)), Qt::QueuedConnection);

    _process->setProgram(_command);
    _process->setArguments(_arguments);
    qDebug().noquote() << "Running command: " << _process->program() << _process->arguments();
    _process->start();
    result = _process->waitForStarted(DEFAULT_TIMEOUT_MS);
    if(result)
    {
        emit clientStarted(_uuid);
    }
    else
    {
        processError();
        goto end;
    }
    if(_requiresPassword)
    {
        QByteArray password( _password.toUtf8() + "\n" );
        _process->write( password.data(), password.size() );
    }
    result = _process->waitForFinished(-1);
    if(result)
    {
        readProcessOutput();
        processFinished();
    }
    else
    {
        processError();
        goto end;
    }
end:    delete _process;
}

void TarsnapCLI::killClient()
{
    if(_process->state() == QProcess::Running)
    {
        _process->terminate();
        if(false == _process->waitForFinished(1000))
            _process->kill();
    }
}

QProcess::ProcessState TarsnapCLI::statusClient()
{
    return _process->state();
}

bool TarsnapCLI::waitForClient()
{
    return _process->waitForFinished(-1);
}

void TarsnapCLI::readProcessOutput()
{
    _processOutput.append(_process->readAll());
}

void TarsnapCLI::processFinished()
{
    QString output(_processOutput);
//    output = output.trimmed();
    qDebug().noquote() << "Command \"" << _command << _arguments
             << "\" finished with return code " << _process->exitCode() << ":"
             << ::endl << output;
    switch (_process->exitStatus()) {
    case QProcess::NormalExit:
        emit clientFinished(_uuid, _process->exitCode(), output);
        break;
    case QProcess::CrashExit:
        processError();
        break;
    }
}

void TarsnapCLI::processError()
{
    qDebug() << "Tarsnap process error " << _process->error() << " occured: "
             << _process->errorString();
}
QUuid TarsnapCLI::uuid() const
{
    return _uuid;
}

void TarsnapCLI::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
}


bool TarsnapCLI::requiresPassword() const
{
    return _requiresPassword;
}

void TarsnapCLI::setRequiresPassword(bool requiresPassword)
{
    _requiresPassword = requiresPassword;
}

QString TarsnapCLI::password() const
{
    return _password;
}

void TarsnapCLI::setPassword(const QString &password)
{
    _password = password;
}





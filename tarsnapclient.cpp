#include "tarsnapclient.h"
#include "debug.h"

#include <QEventLoop>

#define DEFAULT_TIMEOUT_MS 10000

TarsnapClient::TarsnapClient(QUuid uuid) : QObject(), _uuid(uuid), _process(NULL), _requiresPassword(false)
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
    _process->setProcessChannelMode( QProcess::MergedChannels );
//    connect(_process, SIGNAL(started()), this, SIGNAL(clientStarted()), Qt::QueuedConnection);
//    connect(_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()), Qt::QueuedConnection);
//    connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
//            SLOT(processFinished(int, QProcess::ExitStatus)), Qt::QueuedConnection);
//    connect(_process, SIGNAL(error(QProcess::ProcessError)), this,
//             SLOT(processError(QProcess::ProcessError)), Qt::QueuedConnection);

    _process->setProgram(_command);
    _process->setArguments(_arguments);
    LOG << tr("Executing [%1 %2]\n").arg(_process->program()).arg(_process->arguments().join(' '));
    _process->start();
    if(_process->waitForStarted(DEFAULT_TIMEOUT_MS))
    {
        emit clientStarted(_uuid);
    }
    else
    {
        processError();
        goto cleanup;
    }
    if(_requiresPassword)
    {
        QByteArray password( _password.toUtf8() + "\n" );
        _process->write( password.data(), password.size() );
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
    _process = 0;
}

void TarsnapClient::killClient()
{
    if(_process->state() == QProcess::Running)
    {
        _process->terminate();
        if(false == _process->waitForFinished(1000))
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
    if(!output.isEmpty())
        LOG << tr("[%1 %2] finished with exit code %3 and output:\n%4\n").arg(_command).arg(_arguments.join(' ')).arg(_process->exitCode()).arg(output);
    else
        LOG << tr("[%1 %2] finished with exit code %3 and no output.\n").arg(_command).arg(_arguments.join(' ')).arg(_process->exitCode());
    switch (_process->exitStatus())
    {
    case QProcess::NormalExit:
        emit clientFinished(_uuid, _data, _process->exitCode(), output);
        break;
    case QProcess::CrashExit:
        processError();
        break;
    }
}

void TarsnapClient::processError()
{
    LOG << tr("Tarsnap process error %1 (%2) occured:\n%3\n").arg(_process->error()).arg(_process->errorString()).arg(QString(_processOutput));
}
QVariant TarsnapClient::data() const
{
    return _data;
}

void TarsnapClient::setData(const QVariant &data)
{
    _data = data;
}

QUuid TarsnapClient::uuid() const
{
    return _uuid;
}

void TarsnapClient::setUuid(const QUuid &uuid)
{
    _uuid = uuid;
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





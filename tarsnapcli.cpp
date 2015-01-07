#include "tarsnapcli.h"

TarsnapCLI::TarsnapCLI(QObject *parent) : QThread(parent)
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

}




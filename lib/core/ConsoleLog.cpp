#include <ConsoleLog.h>

WARNINGS_DISABLE
#include <QDebug>
#include <QDir>
#include <QFile>
WARNINGS_ENABLE

ConsoleLog *global_log = nullptr;

void ConsoleLog::destroy()
{
    if(global_log == nullptr)
        return;

    // Clean up.
    delete global_log;
    global_log = nullptr;
}

ConsoleLog::ConsoleLog() : _writeToFile(false)
{
}

void ConsoleLog::saveLogMessage(QString msg)
{
    if(!_writeToFile)
        return;

    QFile logFile(_filename);
    if(!logFile.open(QIODevice::Append | QIODevice::Text)
       && !logFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error saving Console Log message: cannot open log file "
                        + logFile.fileName();
        return;
    }
    logFile.write(QByteArray(msg.toLatin1()));
    logFile.close();
}

void ConsoleLog::setFilename(QString filename)
{
    _filename = filename;
}

QString ConsoleLog::getLogFile()
{
    return _filename;
}

void ConsoleLog::setWriteToFile(bool writeToFile)
{
    _writeToFile = writeToFile;
}

#include "debug.h"

#include <QDir>
#include <QFile>
#include <QSettings>

void ConsoleLog::saveLogMessage(QString msg)
{
    QSettings settings;
    if(!settings.value("app/save_console_log").toBool())
        return;

    QFile logFile(getLogFile());
    if(!logFile.open(QIODevice::Append | QIODevice::Text)
       && !logFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        DEBUG << "Error saving Console Log message: cannot open log file "
                     + logFile.fileName();
        return;
    }
    logFile.write(QByteArray(msg.toLatin1()));
    logFile.close();
}

QString ConsoleLog::getLogFile()
{
    QString logFileUrl;

    QSettings settings;
    QString   appdata = settings.value("app/app_data").toString();
    if(appdata.isEmpty())
    {
        DEBUG << "Error saving Console Log message: app/app_data dir not set.";
        return logFileUrl;
    }

    return logFileUrl = appdata + QDir::separator() + DEFAULT_LOG_FILE;
}

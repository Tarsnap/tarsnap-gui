#include "tasks-utils.h"

WARNINGS_DISABLE
#include <QDir>
#include <QString>
WARNINGS_ENABLE

#include <TSettings.h>

#include "tasks-defs.h"

QString makeTarsnapCommand(QString cmd)
{
    TSettings settings;
    QString   tarsnapDir = settings.value("tarsnap/path", "").toString();
    if(tarsnapDir.isEmpty())
        return cmd;
    else
        return tarsnapDir + QDir::separator() + cmd;
}

QStringList makeTarsnapArgs()
{
    TSettings   settings;
    QStringList args;

    /* Default arguments: --keyfile and --cachedir. */
    QString tarsnapKeyFile = settings.value("tarsnap/key", "").toString();
    if(!tarsnapKeyFile.isEmpty())
        args << "--keyfile" << tarsnapKeyFile;
    QString tarsnapCacheDir = settings.value("tarsnap/cache", "").toString();
    if(!tarsnapCacheDir.isEmpty())
        args << "--cachedir" << tarsnapCacheDir;

    /* Network rate limits: --maxbw-rate-down and --maxbw-rate-up. */
    int download_rate_kbps = settings.value("app/limit_download", 0).toInt();
    if(download_rate_kbps)
    {
        args << "--maxbw-rate-down"
             << QString::number(1024 * quint64(download_rate_kbps));
    }
    int upload_rate_kbps = settings.value("app/limit_upload", 0).toInt();
    if(upload_rate_kbps)
    {
        args << "--maxbw-rate-up"
             << QString::number(1024 * quint64(upload_rate_kbps));
    }

    /* Add --no-default-config. */
    if(settings.value("tarsnap/no_default_config", DEFAULT_NO_DEFAULT_CONFIG)
           .toBool())
        args.prepend("--no-default-config");

    return (args);
}

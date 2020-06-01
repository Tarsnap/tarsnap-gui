#include "tasks/tasks-utils.h"

WARNINGS_DISABLE
#include <QDir>
#include <QList>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QVariant>
WARNINGS_ENABLE

#include "TSettings.h"

#include "tasks/tasks-defs.h"

QString makeTarsnapCommand(const QString &cmd)
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

/*
 * The QVersionNumber class was introduced in Qt 5.6, which is later than
 * our target of Qt 5.2.1.
 */
int versionCompare(const QString &found, const QString &fixed)
{
    int i;

    /* Parse strings. */
    QStringList foundlist = found.split(QRegExp("\\."));
    QStringList fixedlist = fixed.split(QRegExp("\\."));

    /* Sanity check. */
    Q_ASSERT(fixedlist.size() == 3);
    Q_ASSERT(foundlist.size() >= 3);

    /* Handle a final element like "39-head". */
    if(foundlist.last().contains("-"))
    {
        QStringList foundextra = foundlist.takeLast().split("-");
        foundlist.append(foundextra);
    }

    /* Append extra 0s to the "fixed" string. */
    for(i = 0; i < foundlist.size() - fixedlist.size(); i++)
        fixedlist.append("0");

    /* Numerically compare each portion of the strings. */
    for(i = 0; i < fixedlist.size(); i++)
    {
        if(foundlist[i].toInt() > fixedlist[i].toInt())
            return (1);
        else if(foundlist[i].toInt() < fixedlist[i].toInt())
            return (-1);
    }

    /* Strings are equal. */
    return (0);
}

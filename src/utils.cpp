#include "utils.h"

WARNINGS_DISABLE
#include <QChar>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QStandardPaths>
#include <QVariant>
WARNINGS_ENABLE

#include <math.h>

#include "TSettings.h"

#include "tasks/tasks-defs.h"

using namespace Utils;

QString Utils::humanBytes(quint64 bytes, int fieldWidth)
{
    TSettings settings;
    bool      IEC  = settings.value("app/iec_prefixes", false).toBool();
    quint64   unit = IEC ? 1024 : 1000;
    if(bytes < unit)
        return QString::number(bytes) + " B";
    int     exp = static_cast<int>(log(bytes) / log(unit));
    QString pre = QString(IEC ? "KMGTPE" : "kMGTPE").at(exp - 1)
                  + QString(IEC ? "i" : "");
    return QString("%1 %2B")
        .arg(static_cast<double>(bytes) / pow(unit, exp), fieldWidth, 'f', 2)
        .arg(pre);
}

#include "humanbytes.h"

WARNINGS_DISABLE
#include <QChar>
#include <QVariant>
WARNINGS_ENABLE

#include <math.h>

#include "TSettings.h"

const QString humanBytes(quint64 bytes, int fieldWidth)
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

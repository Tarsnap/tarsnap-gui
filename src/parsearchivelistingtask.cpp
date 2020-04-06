#include "parsearchivelistingtask.h"

WARNINGS_DISABLE
#include <QThreadPool>
WARNINGS_ENABLE

void ParseArchiveListingTask::run()
{
    QVector<File> files;
    // This splits each line of "tarsnap -tv -f ..." into a QStringList.
    // (We don't actually run "tarsnap -tv", because that data is
    // already stored in the Archive _contents when we created it.)
    QRegExp fileRx("^(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+"
                   "\\s+\\S+\\s+\\S+)\\s+(.+)$");
    for(const QString &line : _listing.split('\n', QString::SkipEmptyParts))
    {
        if(-1 != fileRx.indexIn(line))
        {
            File file;
            file.mode     = fileRx.capturedTexts()[1];
            file.links    = fileRx.capturedTexts()[2].toULongLong();
            file.user     = fileRx.capturedTexts()[3];
            file.group    = fileRx.capturedTexts()[4];
            file.size     = fileRx.capturedTexts()[5].toULongLong();
            file.modified = fileRx.capturedTexts()[6];
            file.name     = fileRx.capturedTexts()[7];
            files.append(file);
        }
    }
    emit result(files);
}

#include "parsearchivelistingtask.h"

WARNINGS_DISABLE
#include <QAtomicInt>
#include <QList>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QVector>
WARNINGS_ENABLE

#include "messages/archivefilestat.h"

#include "compat.h"

ParseArchiveListingTask::ParseArchiveListingTask(const QString &listing)
    : BaseTask(), _listing(listing)
{
    // We don't actually run "tarsnap -tv", because that data is
    // already stored in the Archive _contents when we created it.
}

void ParseArchiveListingTask::run()
{
    QVector<FileStat> files;

    // Parse a line of output from `tarsnap -tv -f ARCHIVENAME`.
    QRegExp lineRx("^(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+"
                   "\\s+\\S+\\s+\\S+)\\s+(.+)$");

    // Check each line.
    for(const QString &line : _listing.split('\n', SKIP_EMPTY_PARTS))
    {
        // Bail if requested.
        if(static_cast<int>(_stopRequested) == 1)
        {
            emit dequeue();
            return;
        }

        // Bail if it doesn't match the expected pattern.
        if(lineRx.indexIn(line) == -1)
            continue;

        FileStat stat;
        stat.mode     = lineRx.capturedTexts()[1];
        stat.links    = lineRx.capturedTexts()[2].toULongLong();
        stat.user     = lineRx.capturedTexts()[3];
        stat.group    = lineRx.capturedTexts()[4];
        stat.size     = lineRx.capturedTexts()[5].toULongLong();
        stat.modified = lineRx.capturedTexts()[6];
        stat.name     = lineRx.capturedTexts()[7];
        files.append(stat);
    }
    emit result(files);
    emit dequeue();
}

void ParseArchiveListingTask::stop()
{
    _stopRequested = 1;
}

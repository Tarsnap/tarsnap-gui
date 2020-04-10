#ifndef PARSEARCHIVELISTINGTASK_H
#define PARSEARCHIVELISTINGTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAtomicInt>
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QVector>
WARNINGS_ENABLE

#include "archivefilestat.h"

/*!
 * \ingroup background-tasks
 * \brief The ParseArchiveListingTask extracts the list of files
 * from an archive.
 */
class ParseArchiveListingTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    //! Constructor.
    //! \param listing the output of <tt>tarsnap -tv</tt>.
    explicit ParseArchiveListingTask(const QString &listing);
    //! Run this task in the background; will emit the \ref result
    //! signal when finished.
    void run();

    //! We want to stop the task.
    void stop();

signals:
    //! The list of files.
    void result(QVector<FileStat> files);

private:
    QString _listing;

    QAtomicInt _stopRequested;
};

#endif /* !PARSEARCHIVELISTINGTASK_H */

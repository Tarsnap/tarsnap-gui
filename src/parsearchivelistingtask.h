#ifndef PARSEARCHIVELISTINGTASK_H
#define PARSEARCHIVELISTINGTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAtomicInt>
#include <QObject>
#include <QString>
#include <QVector>
WARNINGS_ENABLE

#include "messages/archivefilestat.h"

#include "basetask.h"

/*!
 * \ingroup background-tasks
 * \brief The ParseArchiveListingTask extracts the list of files
 * from an archive.
 */
class ParseArchiveListingTask : public BaseTask
{
    Q_OBJECT

public:
    //! Constructor.
    //! \param listing the output of <tt>tarsnap -tv</tt>.
    explicit ParseArchiveListingTask(const QString &listing);
    //! Run this task in the background; will emit the \ref result
    //! signal when finished.
    void run() override;

    //! We want to stop the task.
    void stop() override;

signals:
    //! The list of files.
    void result(QVector<FileStat> files);

private:
    QString _listing;

    QAtomicInt _stopRequested;
};

#endif /* !PARSEARCHIVELISTINGTASK_H */

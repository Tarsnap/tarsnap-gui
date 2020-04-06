#ifndef PARSEARCHIVELISTINGTASK_H
#define PARSEARCHIVELISTINGTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QVector>
WARNINGS_ENABLE

//! Metadata about a file.
struct FileStat
{
    //! Filename
    QString name;
    //! Date-time last modified
    QString modified;
    //! Filesize
    quint64 size;
    //! Owner's user name
    QString user;
    //! Owner's group name
    QString group;
    //! Permissions
    QString mode;
    //! Number of links
    quint64 links;
};

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
    explicit ParseArchiveListingTask(const QString &listing) : _listing(listing)
    {
    }
    //! Run this task in the background; will emit the \ref result
    //! signal when finished.
    void run();

signals:
    //! The list of files.
    void result(QVector<FileStat> files);

private:
    QString _listing;
};

#endif /* !PARSEARCHIVELISTINGTASK_H */

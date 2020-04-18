#ifndef BACKENDDATA_H
#define BACKENDDATA_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "messages/archiveptr.h"
#include "messages/backuptaskdataptr.h"
#include "messages/jobptr.h"

/* Forward declaration(s). */
struct archive_list_data;

/*!
 * \ingroup background-tasks
 * \brief The BackendData is a QObject which manages the \ref Job and
 * \ref Archive data.
 */
class BackendData : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    BackendData();

    //! Load the \ref Archive objects from the \ref PersistentStore.
    //! \warning Not implemented!
    bool loadArchives();
    //! Load the \ref Job objects from the \ref PersistentStore.
    //! \warning Not implemented!
    bool loadJobs();

    //! Get the collection of \ref Archive objects.
    QMap<QString, ArchivePtr> archives();
    //! Get the collection of \ref Job objects.
    QMap<QString, JobPtr> jobs();

    //! Get the number of archives.
    quint64 numArchives();

    //! Add a job to the Jobs list.
    void addJob(JobPtr job);
    //! Delete a Job, and potentially all associated Archives.
    void deleteJob(JobPtr job);

    //! Remove the archives.
    void removeArchives(QList<ArchivePtr> archives);
    //! Replace the stored archives with the list.
    QList<ArchivePtr>
    setArchivesFromList(QList<struct archive_list_data> metadatas);

    //! Create a new Archive based on the BackupTaskData.
    //! \param backupTaskData metadata about the archiving command.
    //! \param truncated was this archive interrupted (i.e.  `.part`).
    ArchivePtr newArchive(BackupTaskDataPtr backupTaskData, bool truncated);
    //! Search for all matching Archive objects which were created by a Job.
    //! \param jobPrefix prefix of the Archive names to match.
    QList<ArchivePtr> findMatchingArchives(const QString &jobPrefix);

private slots:
    //! Load the list of archives belonging to a specific Job (specified
    //! via Qt's `sender()` function call).
    void loadJobArchives();

private:
    QMap<QString, ArchivePtr> _archiveMap;
    QMap<QString, JobPtr>     _jobMap;
};

#endif /* !BACKENDDATA_H */

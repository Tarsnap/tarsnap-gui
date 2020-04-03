#ifndef BACKENDDATA_H
#define BACKENDDATA_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QMap>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"

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

    //! Get the collection of \ref Archive objects.
    QMap<QString, ArchivePtr> archives();
    //! Get the collection of \ref Job objects.
    QMap<QString, JobPtr> jobs();

    //! Get the number of archives.
    quint64 numArchives();

private:
    QMap<QString, ArchivePtr> _archiveMap;
    QMap<QString, JobPtr>     _jobMap;
};

#endif /* !BACKENDDATA_H */

#ifndef ARCHIVEFILESTAT_H
#define ARCHIVEFILESTAT_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
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

#endif /* !ARCHIVEFILESTAT_H */

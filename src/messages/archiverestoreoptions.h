#ifndef ARCHIVERESTOREOPTIONS_H
#define ARCHIVERESTOREOPTIONS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

//! Options for restoring files
struct ArchiveRestoreOptions
{
    //! Restore to the original locations.  We must have exactly one out of
    //! (optionRestore, optionRestoreDir, optionTarArchive).
    bool optionRestore = false;
    //! Restore to a specified directory.  We must have exactly one out of
    //! (optionRestore, optionRestoreDir, optionTarArchive).
    bool optionRestoreDir = false;
    //! Restore as a tar archive.  We must have exactly one out of
    //! (optionRestore, optionRestoreDir, optionTarArchive).
    bool optionTarArchive = false;
    //! Don't complain about overwriting existing files.
    bool overwriteFiles = false;
    //! Don't overwrite existing files that are newer.
    bool keepNewerFiles = true;
    //! Preserve file permissions, owner, modes, flags, and ACLs.
    bool preservePerms = false;
    //! Pathname of the output directory or tarball.  Check
    //! (optionRestoreDir, optionTarArchive) to see which this refers to.
    QString path;
    //! Specific file(s) to restore.
    QStringList files;
};

#endif /* !ARCHIVERESTOREOPTIONS_H */

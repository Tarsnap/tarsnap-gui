#ifndef DIR_UTILS_H
#define DIR_UTILS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QFileInfoList>
#include <QString>
WARNINGS_ENABLE

//! A directory name or an error message.
struct DirMessage
{
    //! Directory which contains the binary, or empty.
    QString dirname;
    //! Error message (if applicable).
    QString errorMessage;
};

// if path.isEmpty it will search in $PATH
// if keygenToo it will search for tarsnap-keygen too
// returns the directory where tarsnap resides if found, otherwise empty string
struct DirMessage findTarsnapClientInPath(const QString &path,
                                          bool           keygenToo = false);

// Search for valid tarsnap keys in the supplied path
QFileInfoList findKeysInPath(const QString &path);

//! Return the reason why dirname is not valid, or empty string for success.
const QString validate_writeable_dir(const QString &dirname);

//! Return the reason why filename is not valid, or empty string for success.
const QString validate_readable_file(const QString &filename);

#endif /* !DIR_UTILS_H */

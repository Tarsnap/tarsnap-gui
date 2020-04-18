#ifndef TARSNAPERROR_H
#define TARSNAPERROR_H

/*!
 * \ingroup background-tasks
 * \brief Errors which can occur from a running \c tarsnap as a CmdlineTask.
 */
enum TarsnapError
{
    CacheError,
    FsckError,
    NetworkError
};

#endif /* !TARSNAPERROR_H */

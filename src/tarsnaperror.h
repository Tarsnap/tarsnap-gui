#ifndef ERROR_H
#define ERROR_H

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

#endif // ERROR_H

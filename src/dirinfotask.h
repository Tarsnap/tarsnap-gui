#ifndef DIRINFOTASK_H
#define DIRINFOTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDir>
#include <QObject>
#include <QRunnable>
WARNINGS_ENABLE

namespace Utils
{

/*!
 * \ingroup background-tasks
 * \brief The GetDirInfoTask reads the filesize and count of a directory
 * and its subdirectories.
 */
class GetDirInfoTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    //! Constructor.
    explicit GetDirInfoTask(QDir dir);
    //! Execute the task.
    void run();

signals:
    //! The directory's size and number of files.
    void result(quint64 size, quint64 count);

private:
    QDir _dir;

    quint64 getDirSize(QDir dir);
    quint64 getDirCount(QDir dir);
};
} // namespace Utils

#endif /* !UTIL_H */

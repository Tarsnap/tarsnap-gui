#ifndef DIRINFOTASK_H
#define DIRINFOTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDir>
#include <QObject>
#include <QRunnable>
WARNINGS_ENABLE

/*!
 * \ingroup background-tasks
 * \brief The DirInfoTask reads the filesize and count of a directory
 * and its subdirectories.
 */
class DirInfoTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    //! Constructor.
    explicit DirInfoTask(QDir dir);
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

#endif /* !UTIL_H */

#ifndef DIRINFOTASK_H
#define DIRINFOTASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAtomicInt>
#include <QDir>
#include <QObject>
WARNINGS_ENABLE

#include "basetask.h"

/*!
 * \ingroup background-tasks
 * \brief The DirInfoTask reads the filesize and count of a directory
 * and its subdirectories.
 */
class DirInfoTask : public BaseTask
{
    Q_OBJECT

public:
    //! Constructor.
    explicit DirInfoTask(QDir dir);

    //! Execute the task.
    void run() override;

    //! We want to stop the task.
    void stop() override;

signals:
    //! The directory's size and number of files.
    void result(quint64 size, quint64 count);

private:
    QDir _dir;

    QAtomicInt _stopRequested;
};

#endif /* !DIRINFOTASK_H */

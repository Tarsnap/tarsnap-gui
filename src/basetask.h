#ifndef BASETASK_H
#define BASETASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QRunnable>
#include <QVariant>
WARNINGS_ENABLE

/*!
 * \ingroup background-tasks
 * \brief The BaseTask is a QObject and QRunnable which takes care
 * of basic Task communication.
 */
class BaseTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    //! Constructor.
    explicit BaseTask();
    ~BaseTask();

    //! Run the command previously given.  Blocks until completed (or failed).
    virtual void run() = 0;
    //! If the task is running, attempt to stop it.
    virtual void stop() = 0;

#ifdef QT_TESTLIB_LIB
    void fake();
#endif

signals:
    //! The task was canceled.
    void canceled();
    //! The task failed to start, or finished (with either success or failure).
    void dequeue();

protected:
    //! Caller supplied data.
    QVariant _data;

#ifdef QT_TESTLIB_LIB
    bool _fake;
#endif
};

#endif // !BASETASK_H

#ifndef BASETASK_H
#define BASETASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QRunnable>
#include <QUuid>
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

    //! Run the command previously given.  Blocks until completed (or failed).
    virtual void run() override = 0;
    //! If the task is running, attempt to stop it.
    virtual void stop() = 0;

    //! Get the Uuid.
    QUuid uuid() const;

#ifdef QT_TESTLIB_LIB
    void fake();
#endif

signals:
    //! The task was canceled.
    void canceled();
    //! The task failed to start, or finished (with either success or failure).
    void dequeue();

protected:
    //! Unique ID.
    QUuid _uuid;

#ifdef QT_TESTLIB_LIB
    //! Don't actually run the next task; for testing only.
    bool _fake;
#endif
};

#endif // !BASETASK_H

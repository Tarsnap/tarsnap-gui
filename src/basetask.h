#ifndef BASETASK_H
#define BASETASK_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QRunnable>
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
};

#endif // !BASETASK_H

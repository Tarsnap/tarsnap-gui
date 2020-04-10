#ifndef SCHEDULEWIDGETS_H
#define SCHEDULEWIDGETS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QWidget;

/*!
 * \ingroup widgets-specialized
 * \brief The SchedulingWidets is a QObject which spawns QWidgets
 * as necessary to confirm details about the scheduling support.
 */
class SchedulingWidgets : public QObject
{
    Q_OBJECT

public:
    //! Constructor.
    SchedulingWidgets(QWidget *parent = nullptr);

    //! Enable the scheduling.
    void enableJobSchedulingButtonClicked();
    //! Disable the scheduling.
    void disableJobSchedulingButtonClicked();

private:
    QWidget *_parent;
};

#endif /* !SCHEDULEWIDGETS_H */

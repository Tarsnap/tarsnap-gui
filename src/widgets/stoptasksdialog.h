#ifndef STOPTASKSDIALOG_H
#define STOPTASKSDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QMessageBox>
WARNINGS_ENABLE

/*!
 * \ingroup widgets-specialized
 * \brief The StopTasksDialog is a QMessageBox which gives the
 * user the option to stop running or queued tasks.
 */
class StopTasksDialog : public QMessageBox
{
    Q_OBJECT
public:
    //! Constructor.
    explicit StopTasksDialog(QWidget *parent = nullptr);
};

#endif // !STOPTASKSDIALOG_H

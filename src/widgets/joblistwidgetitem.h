#ifndef JOBLISTWIDGETITEM_H
#define JOBLISTWIDGETITEM_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QListWidgetItem>
#include <QObject>
WARNINGS_ENABLE

#include "messages/jobptr.h"

/* Forward declaration(s). */
namespace Ui
{
class JobListWidgetItem;
}
class QEvent;
class QWidget;

/*!
 * \ingroup widgets-specialized
 * \brief The JobListWidgetItem is a QListWidgetItem which
 * displays information about a job.
 */
class JobListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    //! Constructor.
    //! \param job pointer to the job to display.
    explicit JobListWidgetItem(JobPtr job);
    ~JobListWidgetItem() override;

    //! Returns a widget with a graphical representation of this archive.
    QWidget *widget();

    //! Returns a pointer to the job.
    JobPtr job() const;

    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

signals:
    //! The user requested to create a new archive from this job.
    void requestBackup();
    //! The user requested more information about this job.
    void requestInspect();
    //! The user requested to restore an archive belonging to this job.
    void requestRestore();
    //! The user requested to remove this item from the job list.
    void requestDelete();

protected:
    //! Processes requests to switch language.
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void update();

private:
    Ui::JobListWidgetItem *_ui;
    QWidget               *_widget;
    JobPtr                 _job;

    void setJob(const JobPtr &job);
    void updateKeyboardShortcutInfo();
};

#endif // JOBLISTWIDGETITEM_H

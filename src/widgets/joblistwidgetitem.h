#ifndef JOBLISTWIDGETITEM_H
#define JOBLISTWIDGETITEM_H

#include "persistentmodel/job.h"
#include "ui_joblistwidgetitem.h"

#include <QListWidgetItem>
#include <QObject>

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
    //! \param job: pointer to the job to display.
    explicit JobListWidgetItem(JobPtr job);
    ~JobListWidgetItem();

    //! Returns a widget with a graphical representation of this archive.
    QWidget *widget();

    //! Returns a pointer to the job.
    JobPtr job() const;

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
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void update();

private:
    Ui::JobListWidgetItem _ui;
    QWidget *             _widget;
    JobPtr                _job;

    void setJob(const JobPtr &job);
    void updateUi();
};

#endif // JOBLISTWIDGETITEM_H

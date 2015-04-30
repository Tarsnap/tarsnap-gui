#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include "persistentmodel/job.h"

#include <QWidget>

namespace Ui {
class JobWidget;
}

class JobWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JobWidget(QWidget *parent = 0);
    ~JobWidget();

    JobPtr job() const;
    void setJob(const JobPtr &job);

public slots:
    void save();

signals:
    void cancel();
    void jobAdded(JobPtr job);
    void inspectJobArchive(ArchivePtr archive);
    void deleteJobArchives(QList<ArchivePtr> archives);
    void restoreJobArchive(ArchivePtr archive, ArchiveRestoreOptions options);

protected slots:
    void updateDetails();

private:
    Ui::JobWidget  *_ui;
    JobPtr          _job;
};

#endif // JOBWIDGET_H

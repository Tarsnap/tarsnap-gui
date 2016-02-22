#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include "persistentmodel/job.h"

#include <QWidget>

namespace Ui
{
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
    void saveNew();

signals:
    void collapse();
    void jobAdded(JobPtr job);
    void inspectJobArchive(ArchivePtr archive);
    void deleteJobArchives(QList<ArchivePtr> archives);
    void restoreJobArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void enableSave(bool enable);

protected slots:
    void updateDetails();
    void restoreLatestArchive();
    bool canSaveNew();
    void showArchiveListMenu(const QPoint& pos);

private:
    Ui::JobWidget *_ui;
    JobPtr         _job;
    bool           _saveEnabled;
};

#endif // JOBWIDGET_H

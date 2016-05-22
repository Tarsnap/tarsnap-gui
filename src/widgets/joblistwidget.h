#ifndef JOBLISTWIDGET_H
#define JOBLISTWIDGET_H

#include "joblistitem.h"
#include "persistentmodel/job.h"

#include <QListWidget>

class JobListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit JobListWidget(QWidget *parent = nullptr);
    ~JobListWidget();

signals:
    void displayJobDetails(JobPtr job);
    void backupJob(BackupTaskPtr backup);
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void deleteJob(JobPtr job, bool purgeArchives);

public slots:
    void addJobs(QMap<QString, JobPtr> jobs);
    void backupSelectedItems();
    void selectJob(JobPtr job);
    void inspectJobByRef(QString jobRef);
    void backupAllJobs();
    void addJob(JobPtr job);
    void inspectSelectedItem();
    void restoreSelectedItem();
    void deleteSelectedItem();

private slots:
    void backupItem();
    void inspectItem();
    void restoreItem();
    void deleteItem();

private:
    void execDeleteJob(JobListItem *jobItem);

protected:
    void keyReleaseEvent(QKeyEvent *event);
};

#endif // JOBLISTWIDGET_H

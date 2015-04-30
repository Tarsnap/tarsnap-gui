#ifndef JOBSLISTWIDGET_H
#define JOBSLISTWIDGET_H

#include "persistentmodel/job.h"
#include "taskmanager.h"

#include <QListWidget>

class JobListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit JobListWidget(QWidget *parent = 0);
    ~JobListWidget();

signals:
    void displayJobDetails(JobPtr job);
    void backupJob(BackupTaskPtr backup);
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);

private slots:
    void backupItem();
    void inspectItem();
    void restoreItem();
    void deleteItem();
    void reloadJobs();
    void addJob(JobPtr job);

private:
    QList<JobPtr> getStoredJobs();
};

#endif // JOBSLISTWIDGET_H

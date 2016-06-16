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
    void setFilter(QString regex);

signals:
    void displayJobDetails(JobPtr job);
    void backupJob(JobPtr job);
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void deleteJob(JobPtr job, bool purgeArchives);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void backupItem();
    void inspectItem();
    void restoreItem();
    void deleteItem();

private:
    void execDeleteJob(JobListItem *jobItem);
};

#endif // JOBLISTWIDGET_H

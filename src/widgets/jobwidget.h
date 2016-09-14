#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include "persistentmodel/job.h"
#include "ui_jobwidget.h"

#include <QWidget>

class JobWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JobWidget(QWidget *parent = nullptr);
    ~JobWidget();

    JobPtr job() const;
    void setJob(const JobPtr &job);

public slots:
    void save();
    void saveNew();
    void updateMatchingArchives(QList<ArchivePtr> archives);

signals:
    void collapse();
    void jobAdded(JobPtr job);
    void inspectJobArchive(ArchivePtr archive);
    void deleteJobArchives(QList<ArchivePtr> archives);
    void restoreJobArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void backupJob(JobPtr job);
    void enableSave(bool enable);
    void findMatchingArchives(QString jobPrefix);


protected:
    void changeEvent(QEvent *event);

private slots:
    void updateDetails();
    void restoreButtonClicked();
    void backupButtonClicked();
    bool canSaveNew();
    void showArchiveListMenu(const QPoint &pos);
    void fsEventReceived();
    void showJobPathsWarn();
    void verifyJob();

private:
    Ui::JobWidget  _ui;
    JobPtr         _job;
    bool           _saveEnabled;
    QTimer         _fsEventUpdate;
};

#endif // JOBWIDGET_H

#ifndef JOBLISTWIDGETITEM_H
#define JOBLISTWIDGETITEM_H

#include "persistentmodel/job.h"
#include "ui_joblistwidgetitem.h"

#include <QListWidgetItem>
#include <QObject>

class JobListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit JobListWidgetItem(JobPtr job);
    ~JobListWidgetItem();

    QWidget *widget();

    JobPtr job() const;
    void setJob(const JobPtr &job);

public slots:
    void update();

signals:
    void requestBackup();
    void requestInspect();
    void requestRestore();
    void requestDelete();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::JobListWidgetItem _ui;
    QWidget *_widget;
    JobPtr  _job;
};

#endif // JOBLISTWIDGETITEM_H

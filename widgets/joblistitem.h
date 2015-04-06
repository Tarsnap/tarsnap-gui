#ifndef JOBLISTITEM_H
#define JOBLISTITEM_H

#include "ui_jobitemwidget.h"
#include "job.h"

#include <QObject>
#include <QListWidgetItem>

namespace Ui {
class JobItemWidget;
}

class JobListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    explicit JobListItem(JobPtr job);
    ~JobListItem();

    QWidget *widget();

    JobPtr job() const;
    void setJob(const JobPtr &job);

signals:
    void requestBackup();
    void requestInspect();
    void requestRestore();

public slots:


private:
    Ui::JobItemWidget   _ui;
    QWidget             _widget;
    JobPtr              _job;
};

#endif // JOBLISTITEM_H

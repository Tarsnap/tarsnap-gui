#ifndef JOBLISTITEM_H
#define JOBLISTITEM_H

#include "persistentmodel/job.h"
#include "ui_jobitemwidget.h"

#include <QListWidgetItem>
#include <QObject>

class JobListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit JobListItem(JobPtr job);
    ~JobListItem();

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

private:
    Ui::JobItemWidget _ui;
    QWidget          *_widget;
    JobPtr            _job;
    bool              _useIECPrefixes;
};

#endif // JOBLISTITEM_H

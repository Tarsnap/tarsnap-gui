#ifndef JOBLISTITEM_H
#define JOBLISTITEM_H

#include "persistentmodel/job.h"

#include <QListWidgetItem>
#include <QObject>

namespace Ui
{
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
    void requestDelete();

public slots:
    void update();

private:
    Ui::JobItemWidget *_ui;
    QWidget          *_widget;
    JobPtr            _job;
    bool              _useIECPrefixes;
};

#endif // JOBLISTITEM_H

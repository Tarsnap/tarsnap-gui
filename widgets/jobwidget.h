#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include "job.h"

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
    void save();

signals:
    void cancel();

private:
    void updateDetails();

private:
    Ui::JobWidget  *_ui;
    JobPtr          _job;
};

#endif // JOBWIDGET_H

#ifndef JOBSLISTWIDGET_H
#define JOBSLISTWIDGET_H

#include "jobmanager.h"

#include <QListWidget>

class JobsListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit JobsListWidget(QWidget *parent = 0);
    ~JobsListWidget();

signals:
    void displayJobDetails(JobPtr job);

public slots:

};

#endif // JOBSLISTWIDGET_H

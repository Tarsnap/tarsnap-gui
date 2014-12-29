#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>

class JobManager : public QObject
{
    Q_OBJECT
public:
    explicit JobManager(QObject *parent = 0);
    ~JobManager();

signals:

public slots:
};

#endif // JOBMANAGER_H

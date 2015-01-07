#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QObject>
#include <QThread>

class JobManager : public QObject
{
    Q_OBJECT
public:
    explicit JobManager(QObject *parent = 0);
    ~JobManager();

signals:

public slots:

private:
    QThread _managerThread; // manager runs on a separate thread
};

#endif // JOBMANAGER_H

#ifndef JOB_H
#define JOB_H

#include "taskmanager.h"

#include <QObject>
#include <QUuid>
#include <QUrl>

class Job;

typedef QSharedPointer<Job> JobPtr;

class Job : public QObject
{
    Q_OBJECT
public:
    explicit Job(QObject *parent = 0);
    ~Job();

    QString name() const;
    void setName(const QString &name);

signals:

public slots:

private:

    QUuid               _uuid;
    QString             _name;
    QList<QUrl>         _urls;
    QList<ArchivePtr>   _archives;
};

#endif // JOB_H

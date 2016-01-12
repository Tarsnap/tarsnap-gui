#ifndef JOURNAL_H
#define JOURNAL_H

#include "persistentobject.h"

#include <QObject>

class Journal : public QObject, public PersistentObject
{
    Q_OBJECT

public:
    explicit Journal(QObject *parent = 0);

signals:

protected:
    // From PersistentObject
    void save();
    void load();
    void purge();

public slots:

private:
    QStringList _journal;
};

#endif // JOURNAL_H

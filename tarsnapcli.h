#ifndef TARSNAPCLI_H
#define TARSNAPCLI_H

#include <QObject>

class TarsnapCLI : public QObject
{
    Q_OBJECT
public:
    explicit TarsnapCLI(QObject *parent = 0);
    ~TarsnapCLI();

signals:

public slots:
};

#endif // TARSNAPCLI_H

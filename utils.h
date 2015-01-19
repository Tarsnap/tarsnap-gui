#ifndef UTILS_H
#define UTILS_H

#include <QRunnable>
#include <QDir>

namespace Utils
{

class GetDirInfoTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    GetDirInfoTask(QDir dir);
    void run();

signals:
    void result(qint64 size, qint64 count);

private:
    QDir _dir;

    qint64 getDirSize(QDir dir);
    qint64 getDirCount(QDir dir);
};


} // Utils

#endif // UTILS_H

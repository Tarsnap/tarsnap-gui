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

// Convert an int64 size of bytes to a human readable string using either
// SI(1000) or binary(1024) units (default is binary)
QString humanBytes(qint64 bytes, bool si = false);

} // Utils

#endif // UTILS_H

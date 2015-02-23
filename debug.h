#ifndef DEBUG_H
#define DEBUG_H

#include <QObject>
#include <QString>
#include <QDebug>

#define DEBUG   qDebug().noquote()
#define LOG     Debug::instance()
#define ENDL    "\n"

class Debug : public QObject
{
    Q_OBJECT
public:
    static void initialize();
    static Debug& instance() { static Debug instance; return instance; }
    ~Debug(){}

    inline Debug& operator<<(QChar t) { DEBUG << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(bool t) { DEBUG << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(char t) { DEBUG << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(signed short t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(unsigned short t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(signed int t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(unsigned int t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(signed long t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(unsigned long t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(qint64 t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(quint64 t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(float t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(double t) { DEBUG << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(const char* t) { DEBUG << t; emit message(QString::fromUtf8(t)); return *this; }
    inline Debug& operator<<(const QString& t) { DEBUG << t; emit message(t); return *this; }
    inline Debug& operator<<(const QStringRef& t) { DEBUG << t; emit message(t.toString()); return *this; }
    inline Debug& operator<<(QLatin1String t) { DEBUG << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(const QByteArray& t) { DEBUG << t; emit message(QString(t)); return *this; }

signals:
    void message(const QString message);

public slots:

private:
    explicit Debug():QObject(){}
    Debug(Debug const&);
    void operator=(Debug const&);
};

#endif // DEBUG_H

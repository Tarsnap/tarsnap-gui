#ifndef DEBUG_H
#define DEBUG_H

#include <QObject>
#include <QString>
#include <QDebug>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define DEBUG   qDebug().noquote()
#else
#define DEBUG   qDebug()
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define WARN   qWarning().noquote()
#else
#define WARN   qWarning()
#endif

#define LOG     Debug::instance()
#define ENDL    "\n"
#define DELIMITER "------------------------------------------------------------------------------"

class Debug : public QObject
{
    Q_OBJECT

public:
    static void initialize() {qSetMessagePattern("%{if-debug}%{file}(%{line}): %{endif}%{message}");}
    static Debug& instance() { static Debug instance; return instance; }
    ~Debug(){}

    inline Debug& operator<<(QChar t) { WARN << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(bool t) { WARN << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(char t) { WARN << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(signed short t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(unsigned short t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(signed int t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(unsigned int t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(signed long t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(unsigned long t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(qint64 t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(quint64 t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(float t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(double t) { WARN << t; emit message(QString::number(t)); return *this; }
    inline Debug& operator<<(const char* t) { WARN << t; emit message(QString::fromUtf8(t)); return *this; }
    inline Debug& operator<<(const QString& t) { WARN << t; emit message(t); return *this; }
    inline Debug& operator<<(const QStringRef& t) { WARN << t; emit message(t.toString()); return *this; }
    inline Debug& operator<<(QLatin1String t) { WARN << t; emit message(QString(t)); return *this; }
    inline Debug& operator<<(const QByteArray& t) { WARN << t; emit message(QString(t)); return *this; }

signals:
    void message(const QString message);

public slots:

private:
    // Yes, a singleton
    inline explicit Debug():QObject(){}
    Debug(Debug const&){}
    void operator=(Debug const&){}
};

#endif // DEBUG_H

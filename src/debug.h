#ifndef DEBUG_H
#define DEBUG_H

#include <QDebug>
#include <QObject>
#include <QString>

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define DEBUG qDebug().noquote()
#else
#define DEBUG qDebug()
#endif

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define WARN qWarning().noquote()
#else
#define WARN qWarning()
#endif

#define LOG ConsoleLog::instance()
#define DEFAULT_LOG_FILE "tarsnap.log"

class ConsoleLog : public QObject
{
    Q_OBJECT

public:
    static void initialize()
    {
        qSetMessagePattern("%{if-debug}%{file}(%{line}): %{endif}%{message}");
    }
    static ConsoleLog &instance()
    {
        static ConsoleLog instance;
        return instance;
    }
    ~ConsoleLog() {}

    inline ConsoleLog &operator<<(QChar t)
    {
        WARN << t;
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(bool t)
    {
        WARN << t;
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(char t)
    {
        WARN << t;
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(signed short t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(unsigned short t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(signed int t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(unsigned int t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(signed long t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(unsigned long t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(qint64 t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(quint64 t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(float t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(double t)
    {
        WARN << t;
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(const char *t)
    {
        WARN << t;
        emit message(QString::fromUtf8(t));
        saveLogMessage(QString::fromUtf8(t));
        return *this;
    }
    inline ConsoleLog &operator<<(const QString &t)
    {
        WARN << t;
        emit message(t);
        saveLogMessage(t);
        return *this;
    }
    inline ConsoleLog &operator<<(const QStringRef &t)
    {
        WARN << t;
        emit message(t.toString());
        saveLogMessage(t.toString());
        return *this;
    }
    inline ConsoleLog &operator<<(QLatin1String t)
    {
        WARN << t;
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(const QByteArray &t)
    {
        WARN << t;
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }

    static QString getLogFile();

public slots:

signals:
    void message(const QString message);

private:
    // Yes, a singleton
    inline explicit ConsoleLog() : QObject() {}
    ConsoleLog(ConsoleLog const &) : QObject() {}
    ConsoleLog &operator=(ConsoleLog const &);

    void saveLogMessage(QString msg);
};

#endif // DEBUG_H

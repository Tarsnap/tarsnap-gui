#ifndef DEBUG_H
#define DEBUG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDebug>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#if(QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
#define DEBUG qDebug().noquote()
#else
#define DEBUG qDebug()
#endif

#define LOG ConsoleLog::instance()
#define DEFAULT_LOG_FILE "tarsnap.log"

/*!
 * \ingroup background-tasks
 * \brief The ConsoleLog is a QObject which saves messages to a log file if
 * "app/save_console_log" is true.
 */
class ConsoleLog : public QObject
{
    Q_OBJECT

public:
    //! Sets the qDebug message pattern.
    static void initializeConsoleLog()
    {
#if defined(QT_DEBUG)
        qSetMessagePattern("%{if-debug}%{file}(%{line}): %{endif}%{message}");
#endif
    }
    //! The singleton instance of this class.
    static ConsoleLog &instance()
    {
        static ConsoleLog instance;
        return instance;
    }
    ~ConsoleLog() {}

    //! The log filename.
    static QString getLogFile();

    //! Saves and emits a message.
    //! @{
    inline ConsoleLog &operator<<(QChar t)
    {
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(bool t)
    {
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(char t)
    {
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(signed short t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(unsigned short t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(signed int t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(unsigned int t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(signed long t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(unsigned long t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(qint64 t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(quint64 t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(float t)
    {
        emit message(QString::number(static_cast<double>(t)));
        saveLogMessage(QString::number(static_cast<double>(t)));
        return *this;
    }
    inline ConsoleLog &operator<<(double t)
    {
        emit message(QString::number(t));
        saveLogMessage(QString::number(t));
        return *this;
    }
    inline ConsoleLog &operator<<(const char *t)
    {
        emit message(QString::fromUtf8(t));
        saveLogMessage(QString::fromUtf8(t));
        return *this;
    }
    inline ConsoleLog &operator<<(const QString &t)
    {
        emit message(t);
        saveLogMessage(t);
        return *this;
    }
    inline ConsoleLog &operator<<(const QStringRef &t)
    {
        emit message(t.toString());
        saveLogMessage(t.toString());
        return *this;
    }
    inline ConsoleLog &operator<<(QLatin1String t)
    {
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    inline ConsoleLog &operator<<(const QByteArray &t)
    {
        emit message(QString(t));
        saveLogMessage(QString(t));
        return *this;
    }
    //! @}

signals:
    //! Message sent to the ConsoleLog.
    void message(const QString message);

private:
    // Yes, a singleton
    inline explicit ConsoleLog() : QObject() {}
    ConsoleLog(ConsoleLog const &) : QObject() {}
    ConsoleLog &operator=(ConsoleLog const &);

    void saveLogMessage(QString msg);
};

#endif // DEBUG_H

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

class ConsoleLog;
extern ConsoleLog *global_log;
#define LOG (*global_log)

/*!
 * \ingroup background-tasks
 * \brief The ConsoleLog is a QObject which will emit a message, and can
 * save messages to a log file if desired.
 */
class ConsoleLog : public QObject
{
    Q_OBJECT

public:
    ConsoleLog();

    //! Initialize the global ConsoleLog object.
    static void initializeConsoleLog()
    {
        if(global_log == nullptr)
            global_log = new ConsoleLog();
    }

    static void destroy();

    //! The log filename.
    void setFilename(QString filename);

    //! The log filename.
    QString getLogFile();

    //! Write log messages to a file.
    void setWriteToFile(bool writeToFile);

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
    void saveLogMessage(QString msg);

    QString _filename;
    bool    _writeToFile;
};

#endif // DEBUG_H

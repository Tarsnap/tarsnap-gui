#ifndef CONSOLELOG_H
#define CONSOLELOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QByteArray>
#include <QChar>
#include <QLatin1String>
#include <QObject>
#include <QString>
#include <QStringRef>
WARNINGS_ENABLE

/* Set up global ConsoleLog. */
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
    static void initializeConsoleLog();

    //! Free the global ConsoleLog object.  Can only be called
    //! once per initializeConsoleLog().
    static void destroy();

    //! The log filename.
    void setFilename(const QString &filename);

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
    void message(const QString &message);

private:
    void saveLogMessage(const QString &msg);

    QString _filename;
    bool    _writeToFile;
};

#endif /* CONSOLELOG_H */

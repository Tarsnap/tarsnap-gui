#ifndef TARSNAPCLI_H
#define TARSNAPCLI_H

#include <QThread>
#include <QProcess>

class TarsnapCLI : public QThread
{
    Q_OBJECT
public:
    explicit TarsnapCLI(QObject *parent = 0);
    ~TarsnapCLI();

    QString command() const;
    void setCommand(const QString &command);

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);

    void runCommand();

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void commandFinished(int exitStatus, QString message, QString stdOut, QString stdErr);

public slots:

private:
    QString         _command;
    QStringList     _arguments;
    QProcess        _process;
};

#endif // TARSNAPCLI_H

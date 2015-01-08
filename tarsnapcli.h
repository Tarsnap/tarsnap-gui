#ifndef TARSNAPCLI_H
#define TARSNAPCLI_H

#include <QThread>
#include <QProcess>

#define CMD_TARSNAP "tarsnap"
#define CMD_TARSNAPKEYGEN "tarsnap-keygen"

class TarsnapCLI : public QObject
{
    Q_OBJECT
public:
    explicit TarsnapCLI(QObject *parent = 0);
    ~TarsnapCLI();

    QString command() const;
    void setCommand(const QString &command);

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);

    void killClient();
    QProcess::ProcessState statusClient();

    bool waitForClient();

    QString password() const;
    void setPassword(const QString &password);

    bool requiresPassword() const;
    void setRequiresPassword(bool requiresPassword);

signals:
    void clientFinished(int exitStatus, QString message, QString output);

public slots:
    void runClient();
    void readProcessOutput();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);

private:
    QProcess         _process;
    QByteArray       _processOutput;
    QString          _command;
    QStringList      _arguments;
    QString          _password;
    bool             _requiresPassword;
    QThread          _thread;
};

#endif // TARSNAPCLI_H
